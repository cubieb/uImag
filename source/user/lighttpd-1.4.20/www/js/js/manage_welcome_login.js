$(function () {
    $("#main>.manage>#btn>a").on("click",function () {
        //测试使用的数据，点击确定按钮，不做任何验证，进入首页
        window.location.href = "manage_home_page.html";
        // $.ajax({
        //     type:"POST",
        //     // url:"",//需要服务端的请求的地址
        //     data: "manageLogin=commit" + "&" + $("#main").serialize(),
        //     error:function (response) {
        //         console.log("你输入的登录密码错误");
        //     },
        //     success:function (response) {
        //         //服务器端返回的数据如果是true的话，进入管理界面的首页
        //         if(response == true){
        //             window.location.href = "manage_home_page.html";
        //         }
        //     }
        // })

        return false;
    });
})