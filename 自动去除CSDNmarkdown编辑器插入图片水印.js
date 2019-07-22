//编写CSDN博客自动去除图片水印脚本
(function () {
    'use strict';
    document.addEventListener('keydown',
        function (e) {
            if (e.ctrlKey && e.code == "KeyV") {
                setTimeout(function () {
                    let imgs = document.querySelectorAll(".token.cl.cl-src");
                    imgs.forEach(function (img) {
                        img.innerHTML = img.innerHTML.substring(0,img.innerHTML.indexOf("?"));
                    });
                },2000);
            }
        });
})();