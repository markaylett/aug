// -*- java -*-
// Note: jsshell can be downloaded from:
// http://www.mozilla.org/js/spidermonkey

function createHttpRequest() {
    var obj = null;
    if (window.XMLHttpRequest) {
        obj = new XMLHttpRequest();
    } else if (window.ActiveXObject) {
        try {
            obj = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
                obj = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {
            }
        }
    }
    return obj;
}

function getXml(url, fn) {
    var obj = createHttpRequest();
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

        s += s ? '&' : '?';
        s += id;
        s += '=';
        s += escape(value);
    }
    return s;
}

var log = [];

function Message(type, text) {
    this.type = type;
    this.text = text;
}

function addLog(type, text) {

    log.unshift(new Message(type, text));
    if (10 < log.length)
        log = log.slice(0, 10);
}

function displayLog() {

    var length = log.length;

    var html = '<table><tr><th>type</th><th>message</th></tr>';
    for (var i = 0; i < length; ++i) {
        var message = log[i];
        html += '<tr><td>' + message.type + '</td>';
        html += '<td>' + message.text + '</td></tr>';
    }
    html += '</table>';

    document.getElementById('log').innerHTML = html;
}

function logMessage(type, text) {

    addLog(type, text);
    displayLog();
}

function logXmlMessages(xml) {

    var messages = xml.getElementsByTagName('message');
    var length = messages.length;

    for (var i = 0; i < length; ++i) {
        var message = messages[i];
        addLog(message.getAttribute('type'),
               message.childNodes[0].nodeValue);
    }

    displayLog();
}

function reconf() {
    logMessage('info', 'reconf');
    getXml('service/reconf', logXmlMessages);
}
