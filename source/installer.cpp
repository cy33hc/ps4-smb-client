#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orbis/libkernel.h>
#include <orbis/Bgft.h>
#include <orbis/AppInstUtil.h>
#include <orbis/UserService.h>
#include <dbglogger.h>
#include "installer.h"
#include "util.h"
#include "http_request.h"

#define BGFT_HEAP_SIZE (1 * 1024 * 1024)

static OrbisBgftInitParams s_bgft_init_params;

static bool s_bgft_initialized = false;

namespace INSTALLER
{
	int Init(void)
	{
		int ret;

		if (s_bgft_initialized)
		{
			goto done;
		}

		memset(&s_bgft_init_params, 0, sizeof(s_bgft_init_params));
		{
			s_bgft_init_params.heapSize = BGFT_HEAP_SIZE;
			s_bgft_init_params.heap = (uint8_t *)malloc(s_bgft_init_params.heapSize);
			if (!s_bgft_init_params.heap)
			{
				goto err;
			}
			memset(s_bgft_init_params.heap, 0, s_bgft_init_params.heapSize);
		}

		dbglogger_log("before sceBgftServiceIntInit");
		ret = sceBgftServiceIntInit(&s_bgft_init_params);
		if (ret)
		{
			dbglogger_log("error sceBgftServiceIntInit");
			goto err_bgft_heap_free;
		}
		dbglogger_log("after sceBgftServiceIntInit");

		s_bgft_initialized = true;

	done:
		return 0;

	err_bgft_heap_free:
		if (s_bgft_init_params.heap)
		{
			free(s_bgft_init_params.heap);
			s_bgft_init_params.heap = NULL;
		}

		memset(&s_bgft_init_params, 0, sizeof(s_bgft_init_params));

	err:
		s_bgft_initialized = false;

		return -1;
	}

	void Exit(void)
	{
		int ret;

		if (!s_bgft_initialized)
		{
			return;
		}

		ret = sceBgftServiceIntTerm();

		if (s_bgft_init_params.heap)
		{
			free(s_bgft_init_params.heap);
			s_bgft_init_params.heap = NULL;
		}

		memset(&s_bgft_init_params, 0, sizeof(s_bgft_init_params));

		s_bgft_initialized = false;
	}

	int InstallPkg(const char *ffilename, pkg_header *header)
	{
		int ret;
		char filepath[2000];
		std::string filename = std::string(ffilename);
		sprintf(filepath, "http://127.0.0.1:9090/%s", Request::UrlEncode(Util::Trim(filename, "/")).c_str());
		std::string cid = std::string((char *)header->pkg_content_id);
		cid = cid.substr(cid.find_first_of("-") + 1, 9);
		int user_id;
		ret = sceUserServiceGetForegroundUser(&user_id);
		const char *package_type;
		uint32_t content_type = BE32(header->pkg_content_type);
		dbglogger_log("content_type=%X", content_type);
		switch (content_type)
		{
		case PKG_CONTENT_TYPE_GD:
			package_type = "PS4GD";
			break;
		case PKG_CONTENT_TYPE_AC:
			package_type = "PS4AC";
			break;
		case PKG_CONTENT_TYPE_AL:
			package_type = "PS4AL";
			break;
		case PKG_CONTENT_TYPE_DP:
			package_type = "PS4DP";
			break;
		default:
			package_type = NULL;
			return -1;
			break;
		}

		dbglogger_log("filepath=%s, content_id=%s", filepath, cid.c_str());
		OrbisBgftDownloadParam params;
		memset(&params, 0, sizeof(params));
		{
			params.userId = user_id;
			params.entitlementType = 5;
			params.id = (char *)header->pkg_content_id;
			params.contentUrl = filepath;
			params.contentName = cid.c_str();
			params.iconPath = "";
			params.playgoScenarioId = "0";
			params.option = ORBIS_BGFT_TASK_OPT_DISABLE_CDN_QUERY_PARAM;
			params.packageType = package_type;
			params.packageSubType = "";
			params.packageSize = BE64(header->pkg_size);
		}

		int task_id = -1;
		dbglogger_log("before sceBgftServiceIntDownloadRegisterTask");
		ret = sceBgftServiceIntDownloadRegisterTask(&params, &task_id);
		if (ret)
		{
			dbglogger_log("err sceBgftServiceIntDownloadRegisterTask ret=0x%08X", ret);
			goto err;
		}

		dbglogger_log("before sceBgftServiceDownloadStartTask");
		ret = sceBgftServiceDownloadStartTask(task_id);
		if (ret)
		{
			dbglogger_log("err sceBgftServiceDownloadStartTask ret=%d", ret);
			goto err;
		}

		return 0;

	err:
		return -1;
	}

}