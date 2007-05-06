// -*- java -*-

function Event(name, spec, tz, next) {
    this.name = name;
    this.spec = spec;
    this.tz = tz;
    this.next = next;
}

function Events(div, log) {

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

        var html = '<table width="100%">'
            + '<tr class="header"><th align="left">name</th>'
            + '<th align="left">spec</th><th align="left">tz</th>'
            + '<th align="left">next</th></tr>';

        for (var id in events) {

            var event = events[id];
            if (id == current) {
                html += '<tr class="current">';
            } else {
                html += '<tr class="item" onclick="setCurrent(' + id + ')">';
            }
            html += '<td>' + event.name + '</td>';
            html += '<td>' + event.spec + '</td>';
            html += '<td>' + event.tz + '</td>';
            html += '<td>' + event.next + '</td>';
        }

        html += '</table>';

        html += '<p>';
        html += '<input class="action" type="button" value="delete"'
            + ' onclick="delEvent();">';
        html += '</p>';

        div.innerHTML = html;
    }

    var displayForm = function() {

        var event = events[current];
        if (event == undefined) {
            current = 0;
            event = new Event('', '', 'local', '');
        }

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

    this.setCurrent = function(id) {
        current = id;
        displayTable();
        displayForm();
    }

    this.getCurrent = function() {
        return current;
    }

    this.encode = function() {
        return encodeIds(encodePair('id', current), ['name', 'spec', 'tz']);
    }
}

var log = null;
var events = null;

function reconf() {
    log.add('info', 'reconf');
    getXml('service/reconf', log.addXml);
}

function loadEvents() {
    log.add('info', 'load events');
    getXml('service/sched/events', events.addXml);
}

function delEvent() {
    var id = events.getCurrent();
    if (id) {
        log.add('info', 'del event: ' + id);
        getXml('service/sched/delevent?id=' + escape(id), events.addXml);
    }
}

function putEvent() {
    log.add('info', 'put event');
    getXml('service/sched/putevent?' + events.encode(), events.addXml);
}

function setCurrent(id) {
    log.add('info', 'set current: ' + id);
    events.setCurrent(id);
}

function init() {
    log = new Log(document.getElementById('log'));
    events = new Events(document.getElementById('view'), log);
    loadEvents();
}
