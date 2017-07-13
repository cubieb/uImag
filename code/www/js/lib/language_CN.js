var language = {};
language.CN = {
	TITLE:"",
	//按钮
	BUTTON:{},
	//下拉框
	SELECT:{},
	//页面中呈现的部分
	HTML:{
		kzsz:{
			wifi_name:'扩展网络无线名称',
			wifi_pswd:'无线密码'
		}
	},
	//js文件中打印出来的文字 错误提示 菜单
	JS:{
		kzsz:"上网设置",
        wxsz:"WiFi设置",
		password:"修改管理密码",

		update:"系统升级",
		led:"关闭指示灯",
        misc_reboot:"重新启动",
        rally_default:"恢复出厂设置",
        //菜单
        uploading:"页面正在升级中！",
		checking: "检测中",
        version_check: "版本检测",
        download_update_package: "升级包暂未下载,点击“立即升级”按钮即可马上下载并升级",
        left: "剩余",
        update_now: "最新升级包下载完成，点击“立即升级”按钮即可进行升级",
        comfirm_reboot:"确定立即重启系统吗？",
        rebooting:"页面正在重启中！",
        comfirm_default:"你真的想把系统参数恢复成出厂默认值吗？",
        recover_param:"页面正在恢复参数中！",
		item_null:"该列表条目为空",
		status_btn:["启用","禁用"],
		check_new_version:"发现",
		update_new_version:"版本，是否升级到新版本",

		//验证函数中的错误提示
		range:"范围",
		need_data:"请输入数据",
		exceed_max:"超出最大长度,已自动截短",
		exceed_max_ssid:"设置的WiFi名称长度超过限制",
		comfirm_pwd:"请输入确认密码",
		pwd_differ:"两次输入的密码不同",
		pwd_same: "新密码不能与旧密码相同，请重新输入！",
		non_null_integer:"请重新输入一个非空整数",
		non_numeric_char:"不能含有非数字字符",
		non_null_decimal:"请重新输入一个非空小数",
		non_decimal_char:"不能含有除数字和小数点外的其它字符",
		digital_format_incorrect:"数字格式不正确",
		non_null_string:"请重新输入一个非空字符串",
		not_illegal_char:"不能含有非法字符",
		illegal_char:"非法字符",
		and:"和",
		blank:"空格",
		non_alphanumeric_char:"不能含有字母数字以外的字符",
		pwd_not_empty:"密码不能为空",
		admin_pwd_not_empty:"管理密码不能为空",
		not_chinese:"不能含有中文字符",
		server_addr_not_null:"服务器地址不能为空",
		discover:"发现",
		piece:"个",
		section:"段",
		the:"第",
		s_in:"在",
		not_in:"不在",
		two:"二",
		three:"三",
		four:"四",
        ip_not_null:"IP地址不能为空",
        ip_val_not_null:"IP值不能为空",
        ip_range:"IP值只能在0-255之间",
        ip_num:"IP地址只接受数字",
        ip_broadcast_addr:"IP地址不能为广播地址",
        ip_network_addr:"IP地址不能为网段地址",
        start_end_ip_err:"起始IP大于结束IP",
		ip_incorrect_len:"IP长度不正确", 
		ip_incorrect:"IP不合法",
        ip_reserve_addr:"IP地址为保留地址",
		not_loopback_addr:"IP不能为回环地址",
		not_multicast_addr:"IP不能为组播地址",
        not_lan_ip_addr:"IP地址不能与放大器地址相同",
        not_lan_mask_addr:"IP地址不能为掩码地址",
		firsr_section_not_zero:"IP第一位不能为0",
        four_section_not_zero:"IP第四位不能为0",
		ip_getway_not_same:"IP地址与默认网关不能相同",
		ip_default_getway_in_same_segment:"IP地址与默认网关应在同一网段",
		out_ip_in_ip_same_segment:"外网IP不能和内网IP在同一网段",
		in_ip_out_ip_same_segment:"内网IP不能和外网IP在同一网段",

        getway_not_null:"默认网关不能为空",
        getway_val_not_null:"默认网关不能为空",
        getway_range:"默认网关只能在0-255之间",
        getway_num:"默认网关只接受数字",
        getway_broadcast_addr:"默认网关不能为广播地址",
        getway_network_addr:"默认网关不能为网段地址",
        getway_incorrect_len:"默认网关长度不正确",
        getway_incorrect:"默认网关不合法",
        getway_reserve_addr:"默认网关为保留地址",
        getway_not_loopback_addr:"默认网关不能为回环地址",
        getway_not_multicast_addr:"默认网关不能为组播地址",
        not_lan_getway_addr:"默认网关不能与放大器地址相同",
        getway_not_lan_mask_addr:"默认网关不能为掩码地址",
        getway_firsr_section_not_zero:"默认网关第一位不能为0",
        getway_four_section_not_zero:"默认网关第四位不能为0",
		dhcp_pool_err:"地址池设置错误",

		dns_not_null:"DNS不能为空",
		dns_format_incorrect:"DNS格式不正确",
		port_not_null:"端口不能为空",
		port_non_numeric_char:"端口不能含有非数字字符",
		port_range:"端口不能大于65535或小于1",
		mask_err:"子网掩码输入错误",
		mask_format_err:"子网掩码不符合规范",
		mask_first_not_zero:"子网掩码第一位不能为0",
		mac_err:"MAC地址错误",
		mac_not:"MAC地址不能为全",
        mac_broadcast_addr:"MAC地址不能为组播地址",
		mtu_1500_576:"MTU值不能大于1500或小于576",
		mtu_1480_576:"MTU值不能大于1480或小于576",
		mtu_1440_576:"MTU值不能大于1440或小于576",
		pppoe_out_time_range:"超时时间应该在1到30之间",
		fragment_out_range:"分片阀值范围在256到2346之间",
		RTSThreshold_out_range:"RTS阀值范围在256到2347之间",
		year_lt_2008:"年份不能小于2008",
		month_range:"月份范围为1-12",
		day_range:"日期范围为1-",
		hour_range:"小时范围为0-23",
		minute_range:"分钟范围为0-59",
		second_range:"秒范围为0-59",
		calendar_not_null:"日期不能为空",
		calendar_format_err:"日期格式不正确",
		year_err:"年份输入不正确",
		month_err:"月份输入不正确",
		day_err:"日期输入不正确",
		url_not_null:"请重新输入一个非空网址",
		url_err:"网址不正确",
		eq_5:"请输入5个字符",
		eq4_20:"请输入(4-20)个字符",
		eq6_20:"请输入(6-20)个字符",
		eq8_63:"请输入(8-63)个字符",
		eq8_30:"请输入(8-30)个字符",
		eq8_31:"请输入(8-31)个字符",
		eq8_32:"请输入(8-32)个字符",
		eq8_64:"请输入(8-64)个字符",
		eq5_26:"请输入(5-26)个字符",
		eq5_31:"请输入(5-31)个字符",
		pwd_low:"弱",
		pwd_medium:"中",
		pwd_strong:"强",
		whole_day:"关闭",
		week:"星期",
		day0:"星期一",
		day1:"星期二",
		day2:"星期三",
		day3:"星期四",
		day4:"星期五",
		day5:"星期六",
		day6:"星期日"
	}
};

var dialog_text = {
	"default":["确认操作","确定","取消"],
	"update":["升级版本","确定","取消"]
};

//other
//auto_fade参数：提示框是否自动消失
var message_panel =message_panel|| new Object();
message_panel = {
	save:{
		type:"wait",
		auto_fade:false,
		message:"正在保存参数，请稍候……"
	},
	refreshing:{
		type:"wait",
		auto_fade:false,
		message:"刷新中，请稍候……"
	},
	deleteing:{
		type:"wait",
		auto_fade:false,
		message:"正在删除，请稍候……"
	},
	delete_err:{
		type:"error",
		auto_fade:true,
		message:"删除失败"
	},
	success:{
		type:"success",
		auto_fade:true,
		message:"设置成功"
	},
    add_success:{
        type:"success",
        auto_fade:true,
        message:"添加成功"
    },
    del_success:{
        type:"success",
        auto_fade:true,
        message:"删除成功"
    },
	quick_success:{
		type:"success",
		auto_fade:false,
		message:"设置成功"
	},
	wait_one_min:{
		type:"wait",
		auto_fade:false,
		message:"请稍候"
	},
	wait:{
		type:"wait",
		auto_fade:false,
		message:"请稍候"
	},
	need_file_name:{
		type:"error",
		auto_fade:true,
		message:"请选择升级文件"
	},
	file_uploading:{
		type:"wait",
		auto_fade:false,
		message:"文件上传中"
	},
	exception:{
		type:"error",
		auto_fade:true,
		message:"异常错误"
	},
    file_upload_success:{
        type:"success",
        auto_fade:true,
        message:"升级成功"
    },
	error:{
		type:"error",
		auto_fade:true,
		message:"设置失败"
	},
	login_failure:{
		type:"error",
		auto_fade:true,
		message:"登录失败"
	},
    msg_info:{
        type:"msg-info",
        auto_fade:true,
        message:"消息提示"
    },
    login_out_wait:{
        type:"wait",
        auto_fade:false,
        message:"正在退出，请稍候…"
    },
    repeater_success:{
        type:"success",
        auto_fade:true,
        message:"连接上端无线成功"
    },
    repeater_failed:{
        type:"error",
        auto_fade:true,
        message:"连接WiFi扩展器失败"
    },
    repeater_ing:{
        type:"wait",
        auto_fade:false,
        message:"正在连接上端无线..."
    },
    repeater_timeout:{
        type:"error",
        auto_fade:true,
        message:"WiFi扩展器初始化完成"
    },
    switch_channel:{
        type:"wait",
        auto_fade:false,
        message:""
    },
    switch_channel_success:{
        type:"wait",
        auto_fade:true,
        message:""
    },
    switch_channel_error:{
        type:"wait",
        auto_fade:true,
        message:""
    },
    net_exception:{
        type:"error",
        auto_fade:true,
        message:"网络异常错误"
    },
	wep_not_support:{
		type:"error",
		auto_fade:true,
		message:"该加密方式不支持"
	},
	file_type_error:{
		type:"error",
		auto_fade:true,
		message:"文件格式异常,请上传后缀带BIN的文件"
	},
	wep_not_roaming:{
		type:"msg-info",
		auto_fade:true,
		message:"WEP加密方式不支持无线漫游"
	},
	net_fail:{
        type:"error",
        auto_fade:true,
        message:"网络异常"
    },
    net_error:{
        type:"error",
        auto_fade:true,
        message:"网络不给力，请检查网络设置"
    }
};

igd.update_info = {
	0:{
		info:"正在初始化自动升级模块"
	},
    1: {
        info: "正在获取新版本…"
    },
	2: {
        info: "获取新版本信息失败，请稍后重试"
    },
	3: {
        info: "当前已是最新版本，无需更新"
    },
	4: {
        info: "有新版本可升级"
    },
	5: {
        info: "有新版本可升级, 正在升级固件"
    },
	6: {
        info: "升级失败"
    },
	7: {
        info: "升级成功等待重启"
    }
};

var language_type = igd.global_param.language_type;
var L = language[language_type]["JS"];