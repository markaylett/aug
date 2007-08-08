// -*- java -*-
// Note: jsshell can be downloaded from:
// http://www.mozilla.org/js/spidermonkey

// Basic Types.

function Pair(id, value) {
    this.id = id;
    this.value = value;
}

Pair.prototype.encode = function() {
    return this.id + '=' + escape(this.value);
};

Pair.prototype.toString = function() {
    return '{id: ' + this.id + ', value: ' + this.value + '}';
};

function getPairById(id) {
    return new Pair(id, document.getElementById(id).value);
}

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

function getXml(fn, url) {
    var obj = xmlHttpRequest();
    obj.onreadystatechange = function() {
        if (obj.readyState == 4){
            fn(obj.responseXML);
        }
    }
    obj.open('GET', url, true);
    obj.send('');
}

function encodePairs(pairs) {
    return map(function(x) { return x.encode(); }, pairs).join('&');
}

// Logging.

function Message(type, text) {
    this.type = type;
    this.text = text;
}

function Log(div) {

    var log_ = [];

    var add = function(type, text) {

        log_.unshift(new Message(type, text));
        if (5 < log_.length)
            log_ = log_.slice(0, 5);
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
            }, log_);

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

function Pager(refresh, request, max) {

    // Items in current page.

    var items_ = [];

    // Current page offset.

    var offset_ = 0;

    // Current, known total.

    var total_ = 0;

    // Current id.

    var current_ = 0;

    var setCurrentId = function(id) {
        current_ = id ? id : 0;
    };

    var itemsOffset = function(offset, n) {

        offset += n * (max - 1);

        if (offset < 0 || 0 == items_.length)
            offset = 0; // If no items or behind start.
        else if (total_ <= offset)
            offset = total_ - 1; // If beyond end.

        return offset;
    };

    // Public functions.

    this.encodePairs = function(pairs) {
        return encodePairs([new Pair('offset', offset_),
                            new Pair('max', max)].concat(pairs));
    };

    // Callback function used for updating current page.

    this.setItems = function(items, offset, total) {
        items_ = items;
        offset_ = offset;
        total_ = total;
        refresh(items, offset, total);
    };

    // Set current item.

    this.setCurrentId = function(id) {
        setCurrentId(id);
        refresh(items_, offset_, total_);
    };

    // Set current item near to specified id - useful when deleting.

    this.setNearId = function(id) {
        this.setCurrentId(this.getNextId(id) || this.getPrevId(id));
    };

    // Move relative to current page offset.

    this.movePage = function(n) {
        request(itemsOffset(offset_, n), max);
    };

    // Set absolute page.

    this.setPage = function(n) {
        request(itemsOffset(0, n), max);
    };

    // Return current item's id.

    this.getCurrentId = function() {
        return current_;
    };

    // Return id of item prior to specified id, or zero if not known.

    this.getPrevId = function(id) {
        if (id) {
            var x = prevById(items_, id);
            if (x)
                return x.value.id;
        }
        return 0;
    };

    // Return id of item after to specified id, or zero if not known.

    this.getNextId = function(id) {
        if (id) {
            var x = nextById(items_, id);
            if (x)
                return x.value.id;
        }
        return 0;
    };

    // Return current item.

    this.getCurrent = function() {
        return current_ ? getById(pages_, current_) : null;
    };

    // Return zero-based index of current page.

    this.getPage = function() {
        return Math.ceil(offset_ / (max - 1));
    };

    // Return number of pages based on current total.

    this.getPages = function() {
        return Math.ceil(total_ / (max - 1));
    };
}
