$(function () {
    //接收动画插件返回的Jq动画对象
    var $transEnd = $(".progress-ring").loadingRing();
    console.log($transEnd);

    //监听Jq动画对象的transitionend事件，动画完成以后执行一个回调函数callback
    //on事件后面没有最后的false，但是js中的addEventListener有false
    $transEnd.on("transitionend", function () {
        window.location.href = "system_reboot_result.html";
    });
});