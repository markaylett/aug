// -*- java -*-
// Note: jsshell can be downloaded from:
// http://www.mozilla.org/js/spidermonkey

function iterate(fn, xs) {
    for (i = 0; i < xs.length; ++i)
        fn(xs[i]);
}

function riterate(fn, xs) {
    for (i = xs.length; 0 < i; --i)
        fn(xs[i - 1]);
}

function filter(fn, xs) {
    var ys = [];
    iterate(function(x) { if (fn(x)) ys.push(x); }, xs);
    return ys;
}

function map(fn, xs) {
    var ys = [];
    iterate(function(x) { ys.push(fn(x)); }, xs);
    return ys;
}

function fold(fn, x, ys) {
    iterate(function(y) { x = fn(x, y); }, ys);
    return x;
}

function rfold(fn, xs, y) {
    riterate(function(x) { y = fn(x, y); }, xs);
    return y;
}

function prod(xs) {
    return fold(function(x, y) { return x * y; }, 1, xs);
}

function sum(xs) {
    return fold(function(x, y) { return x + y; }, 0, xs);
}

function xmlHttpRequest() {
    var obj = null;
    if (window.XMLHttpRequest) {
        obj = new XMLHttpRequest();
    } else if (window.ActiveXObject) {
        try {
            obj = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            obj = new ActiveXObject("Microsoft.XMLHTTP");
        }
    }
    return obj;
}

function getXml(url, fn) {
    var obj = xmlHttpRequest();
    obj.onreadystatechange = function() {
        if (obj.readyState == 4){
            fn(obj.responseXML);
        }
    }
    obj.open('GET', url, true);
    obj.send('');
}

function urlEncode(ids) {
    var s = '';
    for (var i in ids) {

        var id = ids[i];
        var value = document.getElementById(id).value;

        if (s) s += '&';
        s += id;
        s += '=';
        s += escape(value);
    }
    return s;
}

function Message(type, text) {
    this.type = type;
    this.text = text;
}

function Log() {

    var log = [];

    var add = function(type, text) {

        log.unshift(new Message(type, text));
        if (5 < log.length)
            log = log.slice(0, 5);
    }

    var parseXml = function(xml) {

        iterate(function(x) {

                var type = x.getAttribute('type');
                var text = x.childNodes[0].nodeValue;
                add(type, text);

            }, xml.getElementsByTagName('message'));
    }

    var display = function() {

        var html = '<table><tr><th>type</th><th>message</th></tr>';

        iterate(function(x) {
                html += '<tr><td>' + x.type + '</td>';
                html += '<td>' + x.text + '</td></tr>';
            }, log);

        html += '</table>';

        document.getElementById('log').innerHTML = html;
    }

    this.add = function(type, text) {
        add(type, text);
        display();
    }

    this.addXml = function(xml) {
        parseXml(xml);
        display();
    }
}
