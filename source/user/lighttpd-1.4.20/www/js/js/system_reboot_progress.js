$(function () {
    //接收动画插件返回的Jq动画对象
    var $transEnd = $(".progress-ring").loadingRing();
    console.log($transEnd);

    //监听Jq动画对象的transitionend事件，动画完成以后执行一个回调函数callback
    //on事件后面没有最后的false，但是js中的addEventListener有false
    $transEnd.on("transitionend", function () {
        window.location.href = "system_reboot_result.html";
    });



    $.ajax({
        type: "POST",
        url: "/cgi-bin/sys_setting.cgi",//请求的接口数据，拿到上网的人的信息
        async: false,//锁死，一直等待，直到等到服务端有数据返回的时候，才会执行success或者error
        data: "update_sys=update",
        error: function () {
            console.log("刷入开发板flash......失败！");
        },
        success: function (res) {
            console.log(res);
            console.log("刷入开发板flash......成功！");
            //要将ajax设置为同步才会等待，到达90%的时候一直在那边等待，等到有返回的时候，然后结束等待，进入到下一个界面
            if (res == "updateSuccess") {
                percent = 90;
                percentNum = 90;
                console.log("向服务端请求，一直等待是否要刷入flash的信息");
                var timer1 = setInterval(function () {
                    if (percentNum < 100) {
                        // percentNum = percentNum + 1;
                        $(".progress-ring>.progress-text>.progress-num").text(100);//直接给百分百的值

                        percent = 100;//需要规定这个值
                        duration = 100;//需要规定这个值
                        $target.find('.progress-left').css({
                            'transform': 'rotate(' + percent * 3.6 + 'deg)',
                            '-o-transform': 'rotate(' + percent * 3.6 + 'deg)',
                            '-ms-transform': 'rotate(' + percent * 3.6 + 'deg)',
                            '-moz-transform': 'rotate(' + percent * 3.6 + 'deg)',
                            '-webkit-transform': 'rotate(' + percent * 3.6 + 'deg)',
                            'transition': 'transform ' + duration + 'ms linear',
                            '-o-transition': '-o-transform ' + duration + 'ms linear',
                            '-ms-transition': '-ms-transform ' + duration + 'ms linear',
                            '-moz-transition': '-moz-transform ' + duration + 'ms linear',
                            '-webkit-transition': '-webkit-transform ' + duration + 'ms linear'
                        });

                        var animation = 'toggle ' + (duration * 50 / percent) + 'ms'
                        $target.find('.progress-right').css({
                            'opacity': 1,
                            'animation': animation,
                            'animation-timing-function': 'step-end'
                        });
                        $target.find('.progress-cover').css({
                            'opacity': 0,
                            'animation': animation,
                            'animation-timing-function': 'step-start'
                        });
                    }
                }, 100);
            }
        }
    });
});