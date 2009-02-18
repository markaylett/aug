// -*- java -*-

/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

function Event(id, name, spec, tz, next) {
    this.id = id;
    this.name = name;
    this.spec = spec;
    this.tz = tz;
    this.next = next;
}

function getEventParser(setEvents) {

    return function(xml) {

        var offset = xml.documentElement.getAttribute('offset');
        var total = xml.documentElement.getAttribute('total');
        var items = [];

        iterate(function(i, x) {

                var id = x.getAttribute('id');
                var name = x.getAttribute('name');
                var spec = x.getAttribute('spec');
                var tz = x.getAttribute('tz');
                var next = x.childNodes[0].nodeValue;
                items.push(new Event(id, name, spec, tz, next));

            }, xml.getElementsByTagName('event'));

        if (offset == null)
            offset = 0;

        if (total == null)
            total = items.length;

        setEvents(items, offset, total);
    };
}

function Events(div, log) {

    var pager_ = null;

    var parseXml = function(xml) {

        var offset = xml.documentElement.getAttribute('offset');
        var total = xml.documentElement.getAttribute('total');
        var items = [];

        iterate(function(i, x) {

                var id = x.getAttribute('id');
                var name = x.getAttribute('name');
                var spec = x.getAttribute('spec');
                var tz = x.getAttribute('tz');
                var next = x.childNodes[0].nodeValue;
                items.push(new Event(id, name, spec, tz, next));

            }, xml.getElementsByTagName('event'));

        if (offset == null)
            offset = 0;

        if (total == null)
            total = items.length;

        pager_.setItems(items, offset, total);
    };

    var refreshTable = function(items, offset, total) {

        var current = pager_.getCurrentId();

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
            }, items);

        for (var i = items.length; i < 5; ++i)
            html += '<tr class="empty"><td colspan="4">&nbsp;</td></tr>';

        html += '</table>';

        var prev = pager_.getPrevId(current);
        var next = pager_.getNextId(current);

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

    var refreshForm = function(items, offset, total) {

        var current = pager_.getCurrentId();

        var x = null;
        if (current && (x = getById(items, current)))
            x = x.value;
        else {
            current = 0;
            x = new Event('', '', '', 'local', '');
        }

        document.getElementById('name').value = x.name;
        document.getElementById('spec').value = x.spec;
        document.getElementById('tz').value = x.tz;
    };

    var refresh = function(items, offset, total) {
        refreshTable(items, offset, total);
        refreshForm(items, offset, total);
    };

    var request = function(offset, max) {
        getXml(parseXml, 'service/sched/events?'
               + encodePairs([new Pair('offset', offset),
                              new Pair('max', max)]));
    };

    this.loadEvents = function() {
        log.add('info', 'load events');
        request(0, 5);
    };

    this.delEvent = function(id) {
        log.add('info', 'del event: ' + id);
        pager_.setNearId(id);
        getXml(parseXml, 'service/sched/delevent?'
               + pager_.encodePairs([new Pair('id', id)]));
    };

    this.putEvent = function() {
        log.add('info', 'put event');
        getXml(parseXml, 'service/sched/putevent?'
               + pager_.encodePairs([new Pair('id', pager_.getCurrentId()),
                                     getPairById('name'),
                                     getPairById('spec'),
                                     getPairById('tz')]));
    };

    this.setCurrent = function(id) {
        log.add('info', 'set current: ' + id);
        pager_.setCurrentId(id);
    };

    pager_ = new Pager(refresh, request, 5);
}

var log = null;
var events = null;
var loadEvents = null;
var delEvent = null;
var putEvent = null;
var setCurrent = null;

function init() {
    log = new Log(document.getElementById('log'));
    events = new Events(document.getElementById('view'), log);
    loadEvents = events.loadEvents;
    delEvent = events.delEvent;
    putEvent = events.putEvent;
    setCurrent = events.setCurrent;
    loadEvents();
}

function reconf() {
    log.add('info', 'reconf');
    getXml(log.addXml, 'service/reconf');
}
