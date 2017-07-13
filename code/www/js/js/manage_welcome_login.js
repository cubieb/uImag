$(function () {
    $("#main>.manage>#btn>a").on("click",function () {
        $.ajax({
            type: "POST",
            url: "/cgi-bin/wifi_setting.cgi",
            // wifiData是指被选中的wifi的相关信息
            data:"get_data=data",
            error: function (xhr, textStatus) {
                console.log("data_commit---------失败");
            },
            success: function (response) {
                console.log("data_commit------------成功");
                localStorage.setItem("ChosenWifi", response);
            }
        });

        var objStr = JSON.parse(localStorage.getItem("ChosenWifi"));
        if($("#managePassword").val() == objStr.managePassword){
            window.location.href = "manage_home_page.html";
        }else{
            $("#managePassword").val("");
            $("#managePassword").focus();
        }

        // window.location.href = "manage_home_page.html";

        return false;
    });
})