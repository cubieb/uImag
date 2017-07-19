;
(function ($) {
    $.fn.loadingRing = function () {
        var defaultOpt = {
            trackColor: '#f0f0f0',
            progressColor: '#6ec84e',
            percent: 75,
            duration: 1500
        }; // 默认选项
        // $(this).each(function () {
            var $target = $(this);
            var color = $target.data('color'); // 颜色
            var percent = parseInt($target.data('percent'), 10); // 百分比
            var duration = parseFloat($target.data('duration'), 10) * 1000; // 持续时间
            var trackColor, progressColor;
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

        //让数字动起来只需要让数字动就行了。
        var percentNum = 0;
        var timer = setInterval(function () {
            percentNum = percentNum + 1;

            if (percentNum > 100) {
                percentNum = 100;
                clearInterval(timer);//清除定时器
            }

            console.log(percentNum);

            $(".progress-ring>.progress-text>.progress-num").text(percentNum);
        },150)

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