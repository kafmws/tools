// ==UserScript==
// @name         自动去除CSDN插入图片水印
// @namespace    http://tampermonkey.net/
// @version      0.2
// @description  在编写CSDN博客时，有时插入图片到markdown编辑器里，会自动加上CSDN的水印，本脚本自动去除所有插入图片的水印
//                          并可以使用ctrl+Q快捷键手动去除CSDN水印
// @author       kafm
// @match        https://mp.csdn.net/mdeditor/
// @grant        none
// @connect      www.csdn.net
// @include      *://*.csdn.net/*
// @note         在Tampermonkey中开启脚本即可
// ==/UserScript==

(function () {
    'use strict';
    document.addEventListener('keydown',
        function (e) {
            if (e.ctrlKey) {
                if(e.code == "KeyV" | e.code == "KeyQ") clear(1000);
                if(e.shiftKey && e.code == "KeyG"){
                    document.getElementById("file-image__upload").addEventListener("change", ()=>{clear(1000);});
                }
            }
        });

    window.addEventListener("load",function () {
        console.log("start");
        setTimeout(function () {
            document.querySelectorAll(".navigation-bar__button.button.clearfix")[13].addEventListener("click", function () {
                let uploadButton;
                setTimeout(function () {
                    document.getElementById("file-image__upload").addEventListener("change", ()=>{clear(1000);});
                }, 200);
            });
        },2000);
    });

    function clear(ms) {
        setTimeout(function () {
            let imgs = document.querySelectorAll(".token.cl.cl-src");
            imgs.forEach(function (img) {
                let index = img.innerHTML.indexOf("?");
                if (index != -1) img.innerHTML = img.innerHTML.substring(0, index);
            });
        }, ms);
    }
})();