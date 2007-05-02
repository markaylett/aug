// -*- java -*-

var events = null;

function displayEvents(xml) {

    logXmlMessages(xml);
    displayLog();

    events = xml.getElementsByTagName('event');
    var length = events.length;

    var html = '<table>'
        + '<tr><th>name</th><th>spec</th><th>tz</th><th>next</th></tr>';
    for (var i = 0; i < length; ++i) {
        var event = events[i];
        html += '<tr><td>' + event.getAttribute('name') + '</td>';
        html += '<td>' + event.getAttribute('spec') + '</td>';
        html += '<td>' + event.getAttribute('next') + '</td>';
        html += '<td>' + event.childNodes[0].nodeValue + '</td></tr>';
    }
    html += '</table>';

    document.getElementById('events').innerHTML = html;
}

function getEvents() {
    logMessage('info', 'get events');
    getXml('service/sched/getevents', displayEvents);
}

function putEvent() {
    logMessage('info', 'put event');
    var url = 'service/sched/putevent';
    url += urlEncode(['name', 'spec', 'tz']);
    getXml(url, logXmlMessages);
}
