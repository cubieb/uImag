$(function () {
    var $manaPwd = $("#managePassword");
    var $manaEmpty =  $("#main>.manage").find("p:nth-of-type(1)");

    $manaPwd.on("input",function () {
        if($manaPwd.val() == ""){
            $manaEmpty.get(0).style.display = "none";
        }
    })

    $("#main>.manage>#btn>a").on("click",function () {
        //配置界面提交的或者在管理界面修改以后的的数据从同一的接口"get_data=data"获取
        $.ajax({
            type: "POST",
            url: "/cgi-bin/wifi_setting.cgi",
            // wifiData是指被选中的wifi的相关信息
            data:"get_data=data",
            error: function (xhr, textStatus) {
                console.log("从get_data=data数据接口获取数据---------失败");
            },
            success: function (response) {
                console.log("从get_data=data数据接口获取数据------------成功");
                var dataObj = JSON.parse(response);
                var managePassword = dataObj.managePassword;

                if($manaPwd.val() == "" && $manaPwd.val() != managePassword){
                    $manaEmpty.get(0).style.display = "block";
                    $manaEmpty.text("输入不能为空");
                }

                if($manaPwd.val() == managePassword){
                    window.location.href = "manage_home_page.html";
                }

                if($manaPwd.val() != "" && $manaPwd.val() != managePassword){
                    $manaEmpty.get(0).style.display = "block";
                    $manaEmpty.text("密码输入错误");
                    $manaPwd.val("");
                    $manaPwd.focus();
                }
            }
        });

        return false;
    });
})