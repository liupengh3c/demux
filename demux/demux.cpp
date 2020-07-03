// demux.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
extern "C"
{
#include "libavformat/avformat.h"
}

int main()
{
	const char* filename = "yueh.mp4";
	const char* h264_filename = "yueh.h264";
	const char* aac_filename = "yueh.aac";

	int video_stream_index = -1, audio_stream_index = -1;
	int ret = 0;

	AVFormatContext *in_fmt_ctx = NULL, *h264_fmt_ctx = NULL, *aac_fmt_ctx = NULL;
	AVPacket av_pkt;
	AVStream* in_stream = NULL;
	AVStream* out_stream = NULL;

	// 1、打开文件，并读取文件头
	avformat_open_input(&in_fmt_ctx, filename, NULL, NULL);

	// 2、读取流信息
	avformat_find_stream_info(in_fmt_ctx, NULL);
	av_dump_format(in_fmt_ctx, 0, filename,0);

	// 3、视频文件,为输出文件分配avformatcontext
	avformat_alloc_output_context2(&h264_fmt_ctx, NULL, NULL, h264_filename);

	// 4、音频文件,为输出文件分配avformatcontext
	avformat_alloc_output_context2(&aac_fmt_ctx, NULL, NULL, aac_filename);

	for (size_t i = 0; i < in_fmt_ctx->nb_streams; i++)
	{
		in_stream = in_fmt_ctx->streams[i];
		
		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			out_stream = avformat_new_stream(aac_fmt_ctx, NULL);
			audio_stream_index = i;
		}
		else if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			out_stream = avformat_new_stream(h264_fmt_ctx, NULL);
			video_stream_index = i;
		}
		// 如果没有对out_stream的判断会出现C6011的警告
		if (out_stream)
		{
			avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
		}
	}

	// 打印输出文件信息
	std::cout << "--------------------------------------------------" << std::endl;
	av_dump_format(h264_fmt_ctx, 0, h264_filename, 1);
	av_dump_format(aac_fmt_ctx, 0, aac_filename, 1);

	// 5、分别打开音频和视频文件
	if (avio_open(&h264_fmt_ctx->pb,h264_filename,AVIO_FLAG_WRITE) < 0)
	{
		std::cout << "avio_open create and init avio context error(h264)" << std::endl;
		return -1;
	}
	if (avio_open(&aac_fmt_ctx->pb, aac_filename, AVIO_FLAG_WRITE) < 0)
	{
		std::cout << "avio_open create and init avio context error(aac)" << std::endl;
		return -1;
	}

	// 6、分别写header
	ret = avformat_write_header(h264_fmt_ctx, NULL);
	if (ret < 0)
	{
		std::cout << "avformat_write_header error(h264),err=" << ret <<std::endl;
		return -1;
	}
	ret = avformat_write_header(aac_fmt_ctx, NULL);
	if (ret < 0)
	{
		std::cout << "avformat_write_header error(aac)" << ret << std::endl;
		return -1;
	}

	// 7、读取mp4文件，并分别将音频、视频写入对应的文件
	while (true)
	{
		if (av_read_frame(in_fmt_ctx, &av_pkt) < 0)
		{
			break;
		}

		// 如果是音频
		if (av_pkt.stream_index == audio_stream_index)
		{
			// 从mp4读取到的pkt，视频stream_index=0,音频stream_index=1，但是aac和h264文件中只有一条流，必须设置为0
			av_pkt.stream_index = 0;
			av_interleaved_write_frame(aac_fmt_ctx, &av_pkt);

		}
		else if (av_pkt.stream_index == video_stream_index)
		{
			// 如果是视频
			av_pkt.stream_index = 0;
			av_interleaved_write_frame(h264_fmt_ctx, &av_pkt);
		}

		av_packet_unref(&av_pkt);
	}

	// 8、写文件尾
	av_write_trailer(h264_fmt_ctx);
	av_write_trailer(aac_fmt_ctx);

	// 9、关闭输入、输出文件
	avio_close(h264_fmt_ctx->pb);
	avio_close(aac_fmt_ctx->pb);
	avformat_close_input(&in_fmt_ctx);
	avformat_free_context(h264_fmt_ctx);
	avformat_free_context(aac_fmt_ctx);

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
