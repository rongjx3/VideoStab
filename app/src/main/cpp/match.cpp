#include "videoWriterFFmpeg.h"
#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ProjectName", __VA_ARGS__)

int add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, \
	enum AVCodecID codec_id, int bitrate, int frame_rate, int width, int height)
{
	AVCodecContext *c;
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec)) {
		LOGE("can not find encoder");
		return -1;
	}

	ost->st = avformat_new_stream(oc, *codec);
	if (!ost->st)
	{
        LOGE("avformat new stream error");
		return -1;
	}

	ost->st->id = oc->nb_streams - 1;
	c = ost->st->codec;
	if ((*codec)->type == AVMEDIA_TYPE_VIDEO)
	{
		c->codec_id = codec_id;
		c->bit_rate = bitrate;

		av_opt_set(c->priv_data, "preset", "superfast", 0);
		av_opt_set(c->priv_data, "tune", "zerolatency", 0);

		c->width = width;
		c->height = height;
		{
			AVRational r = { 1, frame_rate };
			ost->st->time_base = r;
		}
		c->time_base = ost->st->time_base;
		c->gop_size = 12;
		c->pix_fmt = AV_PIX_FMT_YUV420P;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)   c->max_b_frames = 2;
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)   c->mb_decision = 2;
	}
	else
		return -1;
	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	return 0;
}


int videoWriterFFmpeg::fill_yuv_image(AVFrame *pict, int width, int height, Mat src) {

    int w, h, ret = av_frame_make_writable(pict);
	if (ret < 0) {
        LOGE("cannot make picture writable");
		return -1;
	}

	if (src.cols != width || src.rows != height)
	{
        LOGE("input image error");
		return -1;
	}

	pFrameBGR->data[0]=src.data;

	sws_scale(img_convert_ctx,
			  (const uint8_t* const*)pFrameBGR->data,
			  pFrameBGR->linesize,
			  0,
			  c->height,
			  pict->data,
			  pict->linesize);

	return 0;
}

int videoWriterFFmpeg::initBGRf()
{
	pFrameBGR = av_frame_alloc();
	int size = avpicture_get_size(AV_PIX_FMT_BGR24, c->width, c->height);
	out_buffer = (uint8_t *)av_malloc(size);
	avpicture_fill((AVPicture *)pFrameBGR, out_buffer, AV_PIX_FMT_BGR24, c->width, c->height);
    return 1;
}

videoWriterFFmpeg::videoWriterFFmpeg()
{
	io = { NULL };
	video_st = { 0 };
	videoFrameIdx = 0;
	fmt = NULL;
	oc = NULL;
	video_codec = NULL;
}

videoWriterFFmpeg::~videoWriterFFmpeg()
{

}

int videoWriterFFmpeg::open(char *out_mp4_file, int bitrate_in, int frame_rate_in, int width, int height)
{
	int flag = 0;
	int ret;
	io.output_file_name = out_mp4_file;
	bitrate = bitrate_in;
	frame_rate = frame_rate_in;
	io.frame_width = width;
	io.frame_height = height;
	av_register_all();
	avformat_alloc_output_context2(&oc, NULL, NULL, io.output_file_name);
	if (!oc) {
        LOGE("Could not deduce output format from file extension: using MPEG.");
		avformat_alloc_output_context2(&oc, NULL, "mpeg", io.output_file_name);
		if (!oc)
		{
			flag = -1;
			goto End;
		}
	}
	fmt = oc->oformat;
	if (fmt->video_codec != AV_CODEC_ID_NONE)
	{
		ret = add_stream(&video_st, oc, &video_codec, fmt->video_codec, bitrate, frame_rate, width, height);
		if (ret != 0)
		{
			flag = -1;
			goto End;
		}

	}
	else
	{
        LOGE("cannot find codec id");
		flag = -1;
		goto End;
	}
	c = video_st.st->codec;
	ret = avcodec_open2(c, video_codec, 0);
	if (ret < 0)
	{
        LOGE("open video_codec failed");
		flag = -1;
		goto End;
	}
	video_st.frame = av_frame_alloc();
	if (!video_st.frame)
	{
        LOGE("alloc picture failed");
		flag = -1;
		goto End;
	}
	video_st.frame->format = c->pix_fmt;
	video_st.frame->width = c->width;
	video_st.frame->height = c->height;
	//ret = av_frame_get_buffer(video_st.frame, 32);
	ret = av_frame_get_buffer(video_st.frame, c->width * c->height);
	if (ret < 0)
	{
        LOGE("alloc buffer failed");
		flag = -1;
		goto End;
	}
	av_dump_format(oc, 0, io.output_file_name, 1);
	if (!(fmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&oc->pb, io.output_file_name, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
            LOGE("cannot open ");
			flag = -1;
			goto End;
		}
	}
	ret = avformat_write_header(oc, 0);
	if (ret < 0)
	{
        LOGE("write header error");
		flag = -1;
		goto End;
	}
    initBGRf();
	img_convert_ctx = sws_getContext(c->width,
									 c->height,
									 AV_PIX_FMT_BGR24,
									 c->width,
									 c->height,
									 c->pix_fmt,
									 SWS_BICUBIC,
									 NULL, NULL, NULL);


End:
	//fclose(stderr);
	return flag;

}

int videoWriterFFmpeg::write(Mat src)
{
    AVFrame *frame = NULL;
    AVPacket pkt = { 0 };
    int got_packet = 0;
    AVRational r = { 1,1 };
    int ret;
    av_init_packet(&pkt);
    ret = av_frame_make_writable(video_st.frame);
    if (ret < 0) {
        LOGE("cannot make picture writable");
        return -1;
    }

    if (fill_yuv_image(video_st.frame, video_st.st->codec->width, video_st.st->codec->height,
                           src) == 0) {
        video_st.frame->pts = video_st.next_pts++;
        frame = video_st.frame;
    } else {
        LOGE("cannot fill yuv images");
        return -1;
    }

    if (frame == NULL)
        return -1;

    ret = avcodec_encode_video2(video_st.st->codec, &pkt, frame, &got_packet);
    if (ret >= 0 && got_packet) {
        av_packet_rescale_ts(&pkt, video_st.st->codec->time_base, video_st.st->time_base);
        pkt.stream_index = video_st.st->index;
        ret = av_interleaved_write_frame(oc, &pkt);
        if (ret < 0)
            return -1;
    }
    videoFrameIdx++;

    return 0;
}

void videoWriterFFmpeg::flush()
{
	av_free(out_buffer);
	av_write_trailer(oc);
	avcodec_close(video_st.st->codec);
	av_frame_free(&video_st.frame);
	av_frame_free(&video_st.tmp_frame);
    av_frame_free(&pFrameBGR);
	sws_freeContext(video_st.sws_ctx);
	swr_free(&video_st.swr_ctx);
	if (!(fmt->flags & AVFMT_NOFILE))  avio_closep(&oc->pb);
	avformat_free_context(oc);
	//fclose(stderr);
}