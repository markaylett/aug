// -*- java -*-

function Event(id, name, spec, tz, next) {
    this.id = id;
    this.name = name;
    this.spec = spec;
    this.tz = tz;
    this.next = next;
}

function Events(div, log) {

    var events = [];
    var current = 0;

    var getPrev = function() {
        if (current) {
            var pair = prevById(events, current);
            if (pair)
                return pair.value.id;
        }
        return null;
    };

    var getNext = function() {
        if (current) {
            var pair = nextById(events, current);
            if (pair)
                return pair.value.id;
        }
        return null;
    };

    var parseXml = function(xml) {

        events = [];

        iterate(function(i, x) {

                var id = x.getAttribute('id');
                var name = x.getAttribute('name');
                var spec = x.getAttribute('spec');
                var tz = x.getAttribute('tz');
                var next = x.childNodes[0].nodeValue;
                events.push(new Event(id, name, spec, tz, next));

            }, xml.getElementsByTagName('event'));
    };

    var displayTable = function() {

        var html = '<table width="100%">'
            + '<tr class="header"><th align="left">name</th>'
            + '<th align="left">spec</th><th align="left">tz</th>'
            + '<th align="left">next</th></tr>';

        iterate(function(i, x) {

                if (x.id == current) {
                    html += '<tr class="current">';
                } else {
                    html += '<tr class="item" onclick="setCurrent('
                        + x.id + ')">';
                }
                html += '<td>' + x.name + '</td>';
                html += '<td>' + x.spec + '</td>';
                html += '<td>' + x.tz + '</td>';
                html += '<td>' + x.next + '</td></tr>';
            }, events);

        for (var i = events.length; i < 10; ++i)
            html += '<tr class="empty"><td colspan="4">&nbsp;</td></tr>';

        html += '</table>';

        var prev = getPrev();
        var next = getNext();

        html += '<p>';
        html += '<input class="action" type="button" value="prev"'
        if (prev)
            html += ' onclick="setCurrent(' + prev + ');"/>';
        else
            html += ' disabled="true"/>';

        html += '<input class="action" type="button" value="next"'
        if (next)
            html += ' onclick="setCurrent(' + next + ');"/>';
        else
            html += ' disabled="true"/>';

        html += '<input class="action" type="button" value="delete"';
        if (current)
            html += ' onclick="delEvent(' + current + ');"/>';
        else
            html += ' disabled="true"/>';
        html += '</p>';

        div.innerHTML = html;
    };

    var displayForm = function() {

        var x = null;
        if (current && (x = getById(events, current)))
            x = x.value;
        else {
            current = 0;
            x = new Event('', '', '', 'local', '');
        }

        document.getElementById('name').value = x.name;
        document.getElementById('spec').value = x.spec;
        document.getElementById('tz').value = x.tz;
    };

    this.addXml = function(xml) {
        log.addXml(xml);
        parseXml(xml);
        displayTable();
        displayForm();
    };

    this.setCurrent = function(id) {
        current = id ? id : 0;
        displayTable();
        displayForm();
    };

    this.setPeer = function(id) {
        var peer = getNext(id) || getPrev(id);
        current = peer ? peer : 0;
    };

    this.getCurrent = function() {
        return current;
    };

    this.encode = function() {
        return encodeIds(encodePair('id', current), ['name', 'spec', 'tz']);
    };
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

function delEvent(id) {
    log.add('info', 'del event: ' + id);
    getXml('service/sched/delevent?id=' + escape(id), events.addXml);
    events.setPeer(id);
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
