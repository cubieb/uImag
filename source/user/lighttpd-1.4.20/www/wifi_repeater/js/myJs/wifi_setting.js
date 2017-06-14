/**
 * Created by hp on 2017/5/17.
 */
$(function () {
    initialFun();
    function initialFun() {
        //我的代码中有模板渲染的问题，在点击下拉按钮之前渲染模板，才能实现toggleClass，为了解决点击出现下拉不能收缩的bug，必须在初始化的时候刷新
        var scanJsonStr = localStorage.getItem('scanWifi');
        console.log(scanJsonStr);

        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
        var html = template("listTmp", scanEntity);
        $('#selectCon').html(html);
        console.log("点击下拉按钮之前渲染本地数据到页面成功");
        //初始化wifi_setting页面的数据
        $("#wifi-list>.wifi-name>.wifi-l").text(scanEntity.Scan[0].ssid);
        //主路由placeholder的提示与所选主路由的wifi同步
        $("#wifi-list>.wifi-password>input[type='text']").attr("placeholder", "请输入" + '"' + scanEntity.Scan[0].ssid + '"' + "的密码");
        //用户没有做选择，使用默认值的情况或者用户做了选择点击，但是没有选中哪一个
        var wifiObj = scanEntity.Scan[0];
        wifiData = "Channel=" + wifiObj.Channel + "&" +
            "ssid=" + wifiObj.ssid + "&" +
            "bssid=" + wifiObj.bssid + "&" +
            "security=" + wifiObj.security + "&" +
            "signal=" + wifiObj.signal + "&" +
            "mode=" + wifiObj.mode + "&" +
            "ext_ch=" + wifiObj.ext_ch + "&" +
            "net_type=" + wifiObj.net_type + "&" +
            "wps=" + wifiObj.wps;
        // localStorage.setItem("wifiData", wifiData);
        console.log("成功获取被点击的wifi的相关信息：" + wifiData);

        //将wifi放大器的wifi名和密码设置得与主路由关联
        ssidName = wifiObj.ssid;
        console.log("获取了被点击的wifi名:" + ssidName);
        $("#wifi-list>.wifi-amplifier>#newWifiName").val(ssidName + "-Plus");

        //设置新Wi-Fi的名称与新Wi-Fi密码保持同步设定初始值
        $("#wifi-list>.amplifier-password>#newPassword").attr("placeholder", "请输入" + ssidName + "-Plus" + "的密码");

        //主路由的密码默认与放大器的密码是在用户输入的时候保持同步一致的
        $("#wifi-list>.wifi-password>input[type='text']").on("input", function () {
            $("#wifi-list>.amplifier-password>#newPassword").val($("#wifi-list>.wifi-password>input[type='text']").val());
        });

        //设置新Wi-Fi的名称与新Wi-Fi密码保持同步
        $("#wifi-list>.wifi-amplifier>#newWifiName").on("input", function () {
            $("#wifi-list>.amplifier-password>#newPassword").attr("placeholder", "请输入" + $("#wifi-list>.wifi-amplifier>#newWifiName").val() + "的密码");
        });
    }

    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    //扫描获取wifi列表信息，并渲染到前端页面
    //点击模板的时候需要进行刷新是因为，那个刷新按钮的数据会产生变化，wifi列表要把这种保存到本地的wifi列表数据同步到新的列表中。
    var listBtn = $("#wifi-list>.wifi-name>.wifi-r>.list-btn");

    listBtn.on("click", function (e) {
        //将第一页跳转过程中加载的数据从HTML5的localStorage中取出来
        // 取值时：把获取到的Json字符串转换回对象
        var scanJsonStr = localStorage.getItem('scanWifi');
        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(scanEntity);
        //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
        var html = template("listTmp", scanEntity);
        $('#selectCon').html(html);
        console.log(html);
        console.log("将模板数据渲染到页面成功");
        //js动态将最后的“其它”插入第四个元素之后
        // $(".selectContainer>ul>li:last-of-type").insertAfter($(".selectContainer>ul>li:nth-of-type(4)"));
        e.stopPropagation();//不出现事件的原因是浏览器还有默认事件(跟select相关的)。

        $("#selectCon").select();//在selectContainer.js中的一个封装的方法
    });

    //刷新按钮
    //刷新时发送post请求是为了从服务器获取新的wifi列表数据
    $("#wifi-list>.wifi-name>.wifi-r>.refresh-btn").on("click", function (e) {
        $.ajax({
            url: "/cgi-bin/wifi_setting.cgi",
            type: "POST",
            data: "wifiScan=Scan",
            async: false,
            success: function (responseText) {
                var response = JSON.parse(responseText);
                var html = template("listTmp", response);
                $('#selectCon').html(html);
                console.log(html);

                //注意：存进localStorage里面的数据只能是字符串形式的，取也只能是字符串形式取出来
                var responseStr = JSON.stringify(response);
                localStorage.setItem('scanWifi', responseStr);
                console.log(localStorage);
                console.log("刷新按钮获取服务器的wifi列表数据成功");
            }
        });
        e.stopPropagation();//不出现事件的原因是浏览器还有默认事件。
        $(".selectContainer").select();
    });

    //设置放大器的密码与主路由的密码是否一致（默认勾选上，默认一致）
    //监听checkbox
    $("#wifi-form>#wifi-list>.same-password>.cBox").on("click", function () {
        var isBoolean = $(this).find("input[type='checkbox']").prop("checked");
        if (isBoolean) {
            $("#wifi-list>.password-management").hide(300);
        } else {
            $("#wifi-list>.password-management").show(300);
        }
    });

    // 点击下一步按钮，表单验证通过后，提交数据并且跳转到下一页
    $('#wifi-form>#wifi-list>#next-bnt').click(function () {
        var isBoolean = $("#wifi-form").valid();
        if (isBoolean) {
            $.ajax({
                type: "POST",
                url: "/cgi-bin/wifi_setting.cgi",
                data: "wifiCommit=commit" + "&" + wifiData + "&" + $("#wifi-form").serialize(),
                async: true,
                error: function (request) {
                    console.log("提交表单失败服务端返回的数据" + request);
                    console.log("Connection error");
                    console.log("提交表单数据失败");
                },
                success: function (msg) {
                    console.log("提交表单数据成功后服务端返回的数据" + msg);
                    console.log("提交表单数据成功");
                }
            });
            window.location.href = "configuration.html";
        }
    });

    //写校验规则
    $("#wifi-form").validate({
        rules: {
            wifiPassword: {
                emptyCheck: true,
                manageCodeCheck: true,
            },
            // newWifiName: {
            //     emptyCheck: true,
            //     manageCodeCheck: true,
            // },
            newPassword: {
                emptyCheck: true,
                manageCodeCheck: true,
            },
            managePassword: {
                emptyCheck: true,
                manageCodeCheck: true,
            }
        },
        messages: {
            wifiPassword: {
                //这种验证是有顺序的，写在前面的先验证
                emptyCheck: "输入不能为空",
                manageCodeCheck: "有效密码为8-64位字符",
            },
            // newWifiName: {
            //     emptyCheck: "输入不能为空",
            //     manageCodeCheck: "有效密码为8-64位字符",
            // },
            newPassword: {
                emptyCheck: "输入不能为空",
                manageCodeCheck: "有效密码为8-64位字符",
            },
            managePassword: {
                emptyCheck: "输入不能为空",
                manageCodeCheck: "有效密码为8-64位字符",
            }
        },
        //成功验证，label指的是发生错误时那个标签，就是上面例子中的span（element），是p的子元素
        success: function (label) {
            label.parent().parent().css({"border-color": "#ddd"});
        },
        //其中error是字符串，保存了messages中返回的错误信息，
        // element是验证失败的input元素。
        errorPlacement: function (error, element) {
            console.log(element);
            console.log(error);
            error.appendTo(element.siblings("p"));
            element.parent().css({"border-color": "#EE2222"});
        },
    })
});
