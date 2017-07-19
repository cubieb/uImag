$(function () {
    // $("#main>.online-info>.fr>label").on("click", function (event) {
    //     // event.stopPropagation();
    //
    //     // $(this).prop("checked", "true");
    //     // console.log($(this).prop("checked"));
    //     // console.log($("#main>.online-info>.fr>label").prop("checked"));
    //     // console.log($(this).prop("checked"));
    //
    //     //监听几种模式中的哪一种
    // })

    //获取id和checked值
    // var patternId = "openLong";
    // var patternChecked = "true";
    // $("#main>form>.online-info").find("label").on("click", function () {
    //     console.log("我被点击了");
    //     patternChecked = $(this).find("input[type='radio']").prop("checked");
    //     console.log(patternChecked);
    //     patternId = $(this).find("input[type='radio']").attr("id");
    //     console.log(patternId);
    // });
    //
    // var val = $("#closeTimeDetail").val();
    // console.log(val);
    var selectTime = "00:00";
    $("#appTime").val(selectTime);

    var currYear = (new Date()).getFullYear();
    var opt={};
    opt.date = {preset : 'date'};
    //opt.datetime = { preset : 'datetime', minDate: new Date(2012,3,10,9,22), maxDate: new Date(2014,7,30,15,44), stepMinute: 5  };
    opt.datetime = {preset : 'datetime'};
    opt.time = {preset : 'time'};
    opt.default = {
        theme: 'android-ics light', //皮肤样式
        display: 'modal', //显示方式
        mode: 'scroller', //日期选择模式
        lang:'zh',
        startYear:currYear - 10, //开始年份
        endYear:currYear + 10, //结束年份
        //确定：先执行取消在执行确定
        onSelect:function(textVale,inst){ //选中时触发事件
            console.log("我被选中了.....");
            console.log("textVale--"+textVale);
            console.log(this.id);//this表示调用该插件的对象
            // $("#appTime").val(textVale);
        },
        //取消：直接执行取消
        onClose:function(textVale,inst){ //插件效果退出时执行 inst:表示点击的状态反馈：set/cancel
            console.log("我被取消了.....");
            console.log("textVale--"+textVale);
            console.log(this.id);//this表示调用该插件的对象
        }
    };

    $("#appDate").val('').scroller('destroy').scroller($.extend(opt['date'], opt['default']));

    var optDateTime = $.extend(opt['datetime'], opt['default']);
    var optTime = $.extend(opt['time'], opt['default']);

    $("#appDateTime").mobiscroll(optDateTime).datetime(optDateTime);
    $("#appTime").mobiscroll(optTime).time(optTime);
})