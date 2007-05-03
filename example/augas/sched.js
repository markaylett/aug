// -*- java -*-

function Event(name, spec, tz, next) {
    this.name = name;
    this.spec = spec;
    this.tz = tz;
    this.next = next;
}

function Events(log) {

    var events = {};
    var current = 0;

    var parseXml = function(xml) {

        events = {};

        iterate(function(x) {

                var id = x.getAttribute('id');
                var name = x.getAttribute('name');
                var spec = x.getAttribute('spec');
                var tz = x.getAttribute('tz');
                var next = x.childNodes[0].nodeValue;
                events[id] = new Event(name, spec, tz, next);

            }, xml.getElementsByTagName('event'));
    }

    var displayTable = function() {

        var html = '<table><tr><th>name</th><th>spec</th><th>tz</th>'
            + '<th>next</th><th></th></tr>';

        for (var id in events) {

            var event = events[id];
            html += '<tr><td><a href="#" onclick="setEvent(' + id + ')">'
                + event.name + '</a></td>';
            html += '<td>' + event.spec + '</td>';
            html += '<td>' + event.tz + '</td>';
            html += '<td>' + event.next + '</td>';
            html += '<td><a href="#" onclick="delEvent(' + id
                + ')">del</a></td></tr>';
        }

        html += '<tr><td><a href="#" onclick="setEvent(0)">add</a></td>'
            + '<td colspan=\"3\">...</td></tr>';
        html += '</table>';

        document.getElementById('events').innerHTML = html;
    }

    var displayForm = function() {

        var event = events[current];
        if (event == undefined) {
            current = 0;
            event = new Event('', '', 'local', '');
        }

        document.getElementById('id').value = current;
        document.getElementById('name').value = event.name;
        document.getElementById('spec').value = event.spec;
        document.getElementById('tz').value = event.tz;
    }

    this.addXml = function(xml) {
        log.addXml(xml);
        parseXml(xml);
        displayTable();
        displayForm();
    }

    this.setEvent = function(id) {
        current = id;
        displayForm();
    }

    this.urlEncode = function() {
        return urlEncode(['id', 'name', 'spec', 'tz']);
    }
}

var log = new Log();
var events = new Events(log);

function reconf() {
    log.add('info', 'reconf');
    getXml('service/reconf', log.addXml);
}

function loadEvents() {
    log.add('info', 'load events');
    getXml('service/sched/events', events.addXml);
}

function delEvent(id) {
    log.add('info', 'del event: ' + id);
    getXml('service/sched/delevent?id=' + escape(id), events.addXml);
}

function putEvent() {
    log.add('info', 'put event');
    getXml('service/sched/putevent?' + events.urlEncode(), events.addXml);
}

function setEvent(id) {
    log.add('info', 'set event: ' + id);
    events.setEvent(id);
}
