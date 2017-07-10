$(function () {
    $("#main>.manage>#btn>a").on("click",function () {
        var objStr = localStorage.getItem("ChosenWifi");
        if($("#managePassword").val() == objStr.managePassword){
            window.location.href = "manage_home_page.html";
        }

        window.location.href = "manage_home_page.html";

        return false;
    });
})