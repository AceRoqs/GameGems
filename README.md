This is the code that I used as the basis for my articles in the Game
Programming Gems series. I wrote the following articles, both of which were
included in [Best of Game Programming Gems](http://amzn.to/pODKPx).

* Zobrist Hash using the Mersenne Twister, first included in
[Game Programming Gems 4](http://amzn.to/raRvoG) \(2004\).
* Lock\-Free Algorithms, first included in
[Game Programming Gems 6](http://amzn.to/noFiJx) \(2006\).

This code is targetted toward Visual C++ 2010, though it should also work with
GCC. The master branch serves as the up\-to\-date version of this code. The
gems\_bugfix branch contains the code and projects as they were written for the
Gems books, with minor bug fixes as necessary.

The Mersenne Twister code was written in a tutorial style, and is _not_
optimized for speed. C++03 was the latest version of C++ available when this
code was originally written.  C++11 now includes the Mersenne Twister as one
of several new random number libraries available by default.

The Lock\-Free code is reasonably good, though I caution any user against
using any lock\-free algorithms, as that style of code is _exceptionally_
difficult to debug, and often can perform worse than the equivalent code using
locks. It is fun as a mental exercise, but often difficult for many
programmers to avoid the temptation of using it in practice.

As a case\-in\-point, this lock\-free queue does have a bug in
`LockFreeQueue<T>::Add()`. The CAS between \_pTail\->pNext and NULL isn't
correct as that element isn't guaranteed to still be in the queue. I don't
have a fix, and I can't reproduce the issue, though it has been reported on
Xbox 360 and appears to be a bug independent of architecture.

Toby Jones \([www.turbohex.com](http://www.turbohex.com), [ace.roqs.net](http://ace.roqs.net)\)

