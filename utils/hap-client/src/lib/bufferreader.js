import BufferReader from 'buffer-reader';

function indexOf(needle, encoding = 'utf8') {
    if (!(this.constructor === BufferReader)) {
        throw new TypeError();
    }

    let pos = this.buf.indexOf(needle, this.offset, encoding);
    if (pos >= this.offset) {
        return pos - this.offset
    }
    return -1;
}

function mark() {
    this.mark = this.offset;
}

function seekToMark() {
    if (this.mark != null) {
        this.seek(this.mark);
        delete this.mark;
    }
}

function nextLine(ending = '\r\n') {
    let pos, line;
    if ((pos = this::indexOf(ending)) >= 0) {
        line = this.nextString(pos);
        this.move(ending.length);
        return line;
    }
    return null;
}

function remaining() {
    return this.buf.length - this.offset;
}

export {
    indexOf,
    mark,
    seekToMark,
    nextLine,
    remaining
};
