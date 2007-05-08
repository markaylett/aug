// -*- java -*-
// Note: jsshell can be downloaded from:
// http://www.mozilla.org/js/spidermonkey

// Basic Types.

function Pair(id, value) {
    this.id = id;
    this.value = value;
}

Pair.prototype.toString = function() {
    return '{id: ' + this.id + ', value: ' + this.value + '}';
};

// Functional.

function iterate(fn, xs) {
    for (i = 0; i < xs.length; ++i)
        fn(i, xs[i]);
}

function riterate(fn, xs) {
    for (i = xs.length; 0 < i; --i)
        fn(i, xs[i - 1]);
}

function filter(fn, xs) {
    var ys = [];
    iterate(function(i, x) { if (fn(x)) ys.push(x); }, xs);
    return ys;
}

function map(fn, xs) {
    var ys = [];
    iterate(function(i, x) { ys.push(fn(x)); }, xs);
    return ys;
}

function fold(fn, x, ys) {
    iterate(function(i, y) { x = fn(x, y); }, ys);
    return x;
}

function rfold(fn, xs, y) {
    riterate(function(i, x) { y = fn(x, y); }, xs);
    return y;
}

function prod(xs) {
    return fold(function(x, y) { return x * y; }, 1, xs);
}

function sum(xs) {
    return fold(function(x, y) { return x + y; }, 0, xs);
}

function find(fn, xs) {
    try {
        iterate(function(i, x) { if (fn(x)) throw new Pair(i, x); }, xs);
    } catch (e) {
        if (e instanceof Pair) {
            return e;
        }
        throw e;
    }
    return null;
}

// Algorithms.

function getById(xs, id) {
    return find(function(x) { return x.id == id; }, xs);
}

function nextById(xs, id) {
    var x = getById(xs, id);
    if (x) {
        var i = x.id + 1;
        if (i < xs.length)
            return new Pair(i, xs[i]);
    }
    return null;
}

function prevById(xs, id) {
    var x = getById(xs, id);
    if (x) {
        var i = x.id - 1;
        if (0 <= i)
            return new Pair(i, xs[i]);
    }
    return null;
}

// HTTP Request.

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

// URL Encoding.

function encodePair(x, y) {
    return x + '=' + escape(y);
}

function encodeIds(s, ids) {
    iterate(function(i, x) {
            var y = document.getElementById(x).value;
            if (s) s += '&';
            s += encodePair(x, y);
        }, ids);
    return s;
}

// Logging.

function Message(type, text) {
    this.type = type;
    this.text = text;
}

function Log(div) {

    var log = [];

    var add = function(type, text) {

        log.unshift(new Message(type, text));
        if (5 < log.length)
            log = log.slice(0, 5);
    }

    var parseXml = function(xml) {

        iterate(function(i, x) {

                var type = x.getAttribute('type');
                var text = x.childNodes[0].nodeValue;
                add(type, text);

            }, xml.getElementsByTagName('message'));
    }

    var display = function() {

        var html = '<table><tr>'
          + '<th align="left">type</th><th align="left">message</th></tr>';

        iterate(function(i, x) {
                html += '<tr><td>' + x.type + '</td>';
                html += '<td>' + x.text + '</td></tr>';
            }, log);

        html += '</table>';

        div.innerHTML = html;
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
