/*
Copyright (c) 2010, Yahoo! Inc. All rights reserved.
Code licensed under the BSD License:
http://developer.yahoo.com/yui/license.html
version: 3.3.0
build: 3167
*/
YUI.add("classnamemanager",function(C){var B="classNamePrefix",D="classNameDelimiter",A=C.config;A[B]=A[B]||"yui3";A[D]=A[D]||"-";C.ClassNameManager=function(){var E=A[B],F=A[D];return{getClassName:C.cached(function(){var G=C.Array(arguments);if(G[G.length-1]!==true){G.unshift(E);}else{G.pop();}return G.join(F);})};}();},"3.3.0");