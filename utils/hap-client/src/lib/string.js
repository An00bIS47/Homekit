
function* splitGen(delimiter, limit = null) {
    if (limit === 1) return this;
    if (limit <= 0) limit = null;

    let re = delimiter;
    if (typeof delimiter === 'string') {
        re = new RegExp(delimiter);
    }

    if (re.constructor !== RegExp) {
        throw new TypeError("Delimiter must be a RegExp or a string");
    }

    let count = 0, match, buf = this;
    while (
        (limit === null || count < limit - 1)
            && (match = re.exec(buf))
    )
    {
        yield buf.substr(0, match.index);
        count += 1;
        buf = buf.substr(match.index + match[0].length);
    }

    yield buf;
}

function split() {
    return Array.from(splitGen(...arguments));
}

export {
    split,
    splitGen
}
