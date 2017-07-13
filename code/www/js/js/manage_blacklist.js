$(function () {
    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    // $("body>.blacklist-info").css("display", "block");

    //解析通过url传过来的参数boolean
    var url = window.location.href;
    var isBoolean = decodeURI(getUrlVar("isBoolean"));
    console.log(typeof isBoolean);
    console.log(isBoolean);

    // 获取请求url参数数组
    function getUrlVars() {
        var vars = [], hash;
        var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
        for (var i = 0; i < hashes.length; i++) {
            hash = hashes[i].split('=');
            vars.push(hash[0]);
            vars[hash[0]] = hash[1];
        }
        return vars;
    }

    function getUrlVar(name) {
        return getUrlVars()[name];
    }

    //点击的时候通过加载进来的数据判断数据是否为空跳转进入不同的页面。
    //在编码和解码传参的过程中，boolean类型转换为string类型。
    if (isBoolean == "true") {
        //返回数据为空的情况
        $("body>.blacklist-info").css("display", "block");
    }else{
        initialFun();
        //删除黑名单中某一项
        $("#main>.online-info>.fr").on("click", function () {
            var mac = $("#main>.online-info>.fl>.tv-info>p").text();
            var $parent = $(this).parent();
            $.ajax({
                type: "POST",
                url: "/cgi-bin/client_info.cgi",//需要服务端的请求的地址
                data: "delblacklist=delete" + "&" + "mac=" + mac,
                error: function (response) {
                    console.log("删除黑名单数据失败");
                },
                success: function (response) {
                    console.log(response);
                    console.log("删除黑名单数据成功");
                    if (response == "del") {
                        $parent.css("display", "none");
                    }
                }
            })
        });
    }

    function initialFun() {
        var blackonlineJsonStr = localStorage.getItem('blackList');
        var blackonlineEntity = JSON.parse(blackonlineJsonStr);
        console.log(blackonlineEntity);

        var blackP = blackonlineEntity.Black_List;
        console.log(blackP);

        var html = template("blacklistTmp", blackonlineEntity);
        $('#main').html(html);
    }
})