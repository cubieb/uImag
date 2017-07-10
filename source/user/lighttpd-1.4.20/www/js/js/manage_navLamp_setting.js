$(function () {
    $("#main>.online-info>.fr>label").on("click", function (event) {
        event.stopPropagation();

        // $(this).prop("checked", "true");
        // console.log($(this).prop("checked"));
        // console.log($("#main>.online-info>.fr>label").prop("checked"));
        // console.log($(this).prop("checked"));
    })
})