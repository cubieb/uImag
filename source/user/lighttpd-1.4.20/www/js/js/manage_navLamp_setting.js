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
    var patternId = "openLong";
    var patternChecked = "true";
    $("#main>form>.online-info").find("label").on("click", function () {
        console.log("我被点击了");
        patternChecked = $(this).find("input[type='radio']").prop("checked");
        console.log(patternChecked);
        patternId = $(this).find("input[type='radio']").attr("id");
        console.log(patternId);
    });

    var val = $("#closeTimeDetail").val();
    console.log(val);

    // $.ajax({
    //     type: "POST",
    //     url: "/cgi-bin/led_set.cgi",//需要服务端的请求的地址
    //     data: patternId,
    //     error: function (response) {
    //         console.log("提交指示灯数据------失败");
    //     },
    //     success: function (response) {
    //         console.log("提交指示灯数据------成功");
    //     }
    // })
})