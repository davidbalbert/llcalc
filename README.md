llcalc
======

A calculator written in c using a recursive descent parser. llcalc has great error messages.

##Using

```
$ make
$ ./calc
>> foo = 2 * 3
=> 6
>> bar = foo ^ 2
=> 36
>> baz = foo + bar
=> 42
>> 5 + (2^%)
Expected a number, but got a '%'
	5 + (2^%)
	~~~~~~~^
```

You see? Great error messages!

##License

llcalc is licensed under the terms of the [GPLv2](http://www.gnu.org/licenses/gpl-2.0.html).

llcalc contains `khash.h` which is licensed under the terms of the MIT license. More info can be found here: http://attractivechaos.awardspace.com/khash.h.html

llcalc contains [linenoise](https://github.com/antirez/linenoise), which is licensed under the terms of the BSD license. More info can be found in `linenoise.c`.
