$(function () {
    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    var isChosenWifi = $.isEmptyObject(JSON.parse(localStorage.getItem('ChosenWifi')));
    if (!isChosenWifi) {
        //拿到配置界面保存的被选中的wifi信息列表
        var objStr = localStorage.getItem("ChosenWifi");
        obj = JSON.parse(objStr);//此处向服务端请求来的是被选中的wifi的所有信息，是一个对象格式的。
        console.log(obj);
        //初始化路由设置界面的路由名称(Chosen是wifi_setting页面提交过来的，包括配置界面的主路由的所有信息和用户输入的正确密码)
        $("#newWifiName").val(obj.newWifiName);
        //初始化路由设置界面的路由密码（wifiPwd是输入的主路由的密码）
        $("#newPassword").val(obj.newPassword);
        //管理密码
        $("#managePassword").val(obj.newPassword);

        //初始化进入wifi设置界面的时候，wifi密码与主路由密码是否相同的判断
        var OtherChosenStr = JSON.parse(localStorage.getItem('OtherChosen'));
        var isChosen = $.isEmptyObject(OtherChosenStr);
        console.log(isChosen);
        //当路由设置界面没有做提交的时候，或者提交中的数据位空的时候，那么就和配置界面中提供
        if (isChosen) {
            if ($("#newPassword").val() == obj.wifiPwd) {
                $("#samePassword").prop("checked", true);
            } else {
                $("#samePassword").prop("checked", false);
            }
        } else {
            if ($("#newPassword").val() == OtherChosenStr.wifiPwd) {
                $("#samePassword").prop("checked", true);
            } else {
                $("#samePassword").prop("checked", false);
            }
        }

        //设置管理密码和wifi密码是否相同
        // $("#managePassword").prop("checked", obj.wmcheckedVal);
        //设置模式选择
        var objId = $(obj.id);
        console.log(objId);
        objId.prop("checked", true);
    }

    //监听wifi密码与主路由密码是否相同
    $("#samePassword").parent().on("click", function () {
        console.log($(this));
        var isBoolean = $(this).find("input[type='checkbox']").prop("checked");
        console.log(isBoolean);
        if (isBoolean) {
            $("#managePassword").val(obj.newPassword);
        } else {
            $("#newPassword").val(obj.newPassword);
        }
    });

    //监听管理密码和wifi密码是否相同
    $("#managePwd").parent().on("click", function () {
        console.log($(this));
        var isBoolean = $(this).find("input[type='checkbox']").prop("checked");
        console.log(isBoolean);
        if (isBoolean) {
            $(".online-info-managesamePWD").get(0).style.display = "block";
        } else {
            $(".online-info-managesamePWD").get(0).style.display = "none";
            $("#newPassword").val(obj.newPassword);
        }
    });

    var patternId = "throughWall";
    var patternChecked = "true";
    //监听几种模式中的哪一种
    $("#pattern>.online-info-pattern").find("label").on("click", function () {
        patternChecked = $(this).find("input[type='radio']").prop("checked") ;
        console.log(patternChecked);
        patternId = $(this).find("input[type='radio']").attr("id");
        console.log(patternId);
    });

    //写校验规则
    $("#main>form").validate({
        rules: {
            newWifiName: {
                emptyCheck: true,
                minlength: 1,
                maxlength: 26
            },
            newPassword: {
                emptyCheck: true,
                manageCodeCheck: true,
            }
        },
        messages: {
            newWifiName: {
                emptyCheck: "输入不能为空",
            },
            newPassword: {
                emptyCheck: "输入不能为空",
                manageCodeCheck: "有效密码为8-64位字符",
            }
        },
        //成功验证，label指的是发生错误时那个标签，就是上面例子中的span（element），是p的子元素
        success: function (label) {
            // label.parent().parent().css({"border-color": "#ddd"});//暂时去掉
        },
        //其中error是字符串，保存了messages中返回的错误信息，
        // element是验证失败的input元素。
        errorPlacement: function (error, element) {
            console.log(element);
            console.log(error);
            error.appendTo(element.siblings("p"));
            // element.parent().css({"border-color": "#EE2222"});//暂时去掉
        },
        // onfocusout:false
    });

    //确定按钮
    var $btn = $("#main>form>button>a");
    $btn.on("click", function () {
        // 点击下一步按钮，表单验证通过后，提交数据并且跳转到下一页
        var isBoolean = $("#main>form").valid();
        console.log(isBoolean);
        if (isBoolean) {
            //提交当前的路由信息和密码
            // $.ajax({
            //     type: "POST",
            //     url: "/cgi-bin/wifi_setting.cgi",//需要服务端的请求的地址
            //     data: "wifiScan=Scan",//需要哪些数据
            //     error: function (response) {
            //         console.log("提交当前路由设置信息失败");
            //     },
            //     success: function (response) {
            //         console.log("提交当前路由设置信息成功");
            //         console.log(response);
            //     }
            // })

            $.ajax({
                type: "POST",
                url: "/cgi-bin/wifi_setting.cgi",//需要服务端的请求的地址
                data: "ex_station=station" + "&" + "samePasswordChecked=" + $("#samePassword").prop("checked") + "&" + "managePwd=" + $("#managePwd").prop("checked") + "&" + patternId + "=" + patternChecked + "&" + $("#main>form").serialize(),//需要哪些数据
                error: function (response) {
                    console.log("提交当前路由设置信息失败");
                },
                success: function (response) {
                    console.log(response);//请返回wifiSuccess
                    console.log("提交当前wifi设置信息成功");

                    var objCommit = {};
                    objCommit["newWifiName"] = $("#newWifiName").val();
                    objCommit["newPassword"] = $("#newPassword").val();
                    objCommit["wzcheckedVal"] = $("#samePassword").prop("checked");
                    objCommit["wmcheckedVal"] = $("#managePwd").prop("checked");

                    // if ($("#newPassword").val() == obj.wifiPwd) {
                    //     objCommit["wzcheckedVal"] = true;
                    // } else {
                    //     objCommit["wzcheckedVal"] = true;
                    // }
                    //
                    // //wifi密码与管理密码是否相同
                    // objCommit["wmcheckedVal"] = $("#samePassword").prop("checked");
                    //


                    //获取被选中的radio的checked值
                    var $label = $("#pattern .online-info-pattern label");
                    $label.on("click", function () {
                        var $checked = $(this).find("input[type='radio']").prop("checked");
                        if ($checked == true) {
                            var id = $(this).find("input[type='radio']").attr("id");
                            objCommit["id"] = id;
                        }
                    });

                    console.log(objCommit);
                    var objStr = JSON.stringify(objCommit);
                    localStorage.setItem("ChosenWifi", objStr);

                    //后端返回的是"wifiSuccess"
                    if (response == "wifiSuccess") {
                        //执行取消操作，进去重启放大器的过程
                        $.ajax({
                            type: "POST",
                            url: "/cgi-bin/sys_setting.cgi",//请求的接口数据，拿到上网的人的信息
                            data: "reboot_sys=reboot",
                            error: function () {
                                console.log("开始重启系统......失败！");
                            },
                            success: function (response) {
                                console.log("开始重启系统......成功！");
                                console.log(response);
                                if (response == "reboot") {
                                    window.location.href = "syetem_reboot_progress.html";
                                }
                            }
                        });
                    }
                }
            })
        }
        return false;
    })
})