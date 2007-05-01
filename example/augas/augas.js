// -*- java -*-

function getHttpRequest() {
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

function readXml(file, fn) {
    var obj = getHttpRequest();
    obj.onreadystatechange = function() {
        if (obj.readyState == 4){
            fn(obj.responseXML);
        }
    }
    obj.open('GET', file, true);
    obj.send(null);
}

function processXml(obj) {
    var dataArray = obj.getElementsByTagName('event');
    var dataArrayLen = dataArray.length;
    var insertData = '<table style="width:300px; border: solid 1px #000"><tr>'
        + '<th>Name</th><th>Spec</th><th>TZ</th><th>Next</th></tr>';
    for (var i=0; i<dataArrayLen; i++){
        var attrs = dataArray[i].attributes;
        insertData += '<tr><td>' + attrs.getNamedItem('name').value + '</td>';
        insertData += '<td>' + attrs.getNamedItem('spec').value + '</td>';
        insertData += '<td>' + attrs.getNamedItem('tz').value + '</td>';
        insertData += '<td>' + dataArray[i].firstChild.data + '</td></tr>';
    }
    insertData += '</table>';
    document.getElementById ('dataArea').innerHTML = insertData;
}
