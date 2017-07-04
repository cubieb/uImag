/**
 * Created by hp on 2017/5/17.
 */
var wifiData;   //我是全局变量
var ssidName;
var wifiObj;

(function ($) {
    jQuery.fn.select = function () {
        return this.each(function () {
            var $this = $(this);
            var $shows = $this.find(".shows");
            var $selectOption = $this.find(".selectOption");
            var $el = $this.find("ul > li");
            // console.log("111");


            $this.toggleClass("zIndex");
            //我的代码中有模板渲染的问题，在点击下拉按钮之前渲染模板，才能实现toggleClass
            //这是解决点击出现下拉不能收缩的bug
            $this.find("ul").toggleClass("dis");//显示隐藏
            $this.find("#other").toggleClass("dis");//显示隐藏

            $("body").on("click", function () {
                $this.removeClass("zIndex");
                $this.find("ul").removeClass("dis");
                $this.find("#other").removeClass("dis");
            })

            //所有的li
            $el.on("click", function () {
                console.log("li被点击了");
                var $this_ = $(this);
                $("#wifi-list>.wifi-name>.wifi-l").text($this_.text());

                var globalIndex = $this_.index();//点击以后外面访问不到这里的index
                console.log(globalIndex);

                var index =  globalIndex || 0;
                console.log(index);

                wifiObj = scanEntity.Scan[index];
                //获取到被点击wifi的9条相关信息
                wifiData = "Channel=" + wifiObj.Channel + "&" +
                    "ssid=" + wifiObj.ssid + "&" +
                    "bssid=" + wifiObj.bssid + "&" +
                    "security=" + wifiObj.security + "&" +
                    "signal=" + wifiObj.signal + "&" +
                    "mode=" + wifiObj.mode + "&" +
                    "ext_ch=" + wifiObj.ext_ch + "&" +
                    "net_type=" + wifiObj.net_type + "&" +
                    "wps=" + wifiObj.wps;
                console.log("成功获取被点击的wifi的相关信息：" + wifiData);

                //主路由placeholder的提示与所选主路由的wifi同步
                $("#wifi-list>.wifi-password>input[type='text']").attr("placeholder", "请输入"+'"'+scanEntity.Scan[index].ssid+'"'+"的密码")

                //将wifi放大器的wifi名和密码设置得与主路由关联
                ssidName = wifiObj.ssid;
                console.log("成功获取被点击的wifi的相关信息：" + ssidName);
                $("#wifi-list>.wifi-amplifier>#newWifiName").val(ssidName + "-Plus");
                $("#wifi-list>.amplifier-password>#newPassword").attr("placeholder","请输入"+ssidName + "-Plus"+"的密码");
            });
        });
    }
})(jQuery);

