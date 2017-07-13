;
(function ($) {
    $.fn.loadingRing = function () {
        var defaultOpt = {
            trackColor: '#f0f0f0',
            progressColor: '#6ec84e',
            percent: 0,
            duration: 1500
        }; // 默认选项
        // $(this).each(function () {
        var $target = $(this);
        var color = $target.data('color'); // 颜色
        var percent = parseInt($target.data('percent'), 10); // 百分比
        var duration = parseFloat($target.data('duration'), 10) * 1000; // 持续时间
        var trackColor, progressColor,numCount=0;
        if (color && color.split(',').length === 2) {
            var colorSet = color.split(',');
            trackColor = colorSet[0];
            progressColor = colorSet[1];
        } else {
            trackColor = defaultOpt.trackColor;
            progressColor = defaultOpt.progressColor;
        }
        if (!percent)
            percent = defaultOpt.percent;
        if (!duration)
            duration = defaultOpt.duration;

        $target.append('<div class="progress-track"></div><div class="progress-left"></div><div class="progress-right"></div><div class="progress-cover"></div><div class="progress-text"><span class="progress-num">' + percent + '</span><span class="progress-percent">%</span></div>');

        var x = $target.find('.progress-cover').height(); // 触发 Layout
        // http://stackoverflow.com/questions/12088819/css-transitions-on-new-elements

        $target.find('.progress-track, .progress-cover').css('border-color', trackColor);
        $target.find('.progress-left, .progress-right').css('border-color', progressColor);


        //让数字动起来只需要让数字动就行了。
        var percentNum = 0;
        var timer = setInterval(countTimer, 100);

        function countTimer() {
            percentNum = percentNum + 1;

            if (percentNum > 90) {//起到清除倒计时百分比的作用
                percentNum = 90;
                clearInterval(timer);//清除定时器
            }
            console.log(percentNum);

            $(".progress-ring>.progress-text>.progress-num").text(percentNum);

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

            if (percent > 50) {
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

            if (percent >= 90 && percentNum >= 90) {
                numCount++;
                if(numCount == 1){
                    percent = 90;
                    percentNum = 90;
                    console.log("哈哈");
                    $target.find('.progress-left').stop(true, false); //清除当前的动画队列，然后让整个动画停止。

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
                                window.location.href = "system_reboot_result.html";
                            }
                        }
                    });
                }
                // var res = "update_success";
            }

            // if (res == "update_success") {
            //     percent = 90;
            //     percentNum = 90;
            //     setTimeout(function () {
            //         console.log("成功延时4s");
            //         // setInterval(countTimer, 50);//开启定时器
            //         var timer1 = setInterval(function () {
            //             if (percentNum < 100) {
            //                 percentNum = percentNum + 1;
            //                 $(".progress-ring>.progress-text>.progress-num").text(percentNum);
            //
            //                 percent = 100;//需要规定这个值
            //                 duration = 500;//需要规定这个值
            //                 $target.find('.progress-left').css({
            //                     'transform': 'rotate(' + percent * 3.6 + 'deg)',
            //                     '-o-transform': 'rotate(' + percent * 3.6 + 'deg)',
            //                     '-ms-transform': 'rotate(' + percent * 3.6 + 'deg)',
            //                     '-moz-transform': 'rotate(' + percent * 3.6 + 'deg)',
            //                     '-webkit-transform': 'rotate(' + percent * 3.6 + 'deg)',
            //                     'transition': 'transform ' + duration + 'ms linear',
            //                     '-o-transition': '-o-transform ' + duration + 'ms linear',
            //                     '-ms-transition': '-ms-transform ' + duration + 'ms linear',
            //                     '-moz-transition': '-moz-transform ' + duration + 'ms linear',
            //                     '-webkit-transition': '-webkit-transform ' + duration + 'ms linear'
            //                 });
            //
            //                 var animation = 'toggle ' + (duration * 50 / percent) + 'ms'
            //                 $target.find('.progress-right').css({
            //                     'opacity': 1,
            //                     'animation': animation,
            //                     'animation-timing-function': 'step-end'
            //                 });
            //                 $target.find('.progress-cover').css({
            //                     'opacity': 0,
            //                     'animation': animation,
            //                     'animation-timing-function': 'step-start'
            //                 });
            //             }
            //         }, 50);
            //     }, 4000);
            // }
        }

        //给$target增加监听事件，监听动画完成以后要执行的回调函数callback
        //on事件后面没有最后的false，但是js中的addEventListener有false
        // $target.find('.progress-left').on("transitionend", function () {
        //     console.log("动画执行完成");
        // });

        // });

        //去掉循环遍历的each，在结束的时候，返回执行动画tansition的Jq对象
        return $target.find('.progress-left');
    };
})(jQuery);