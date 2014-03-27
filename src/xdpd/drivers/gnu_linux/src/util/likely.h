/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LIKELY_H
#define LIKELY_H 1

#ifndef likely
	#define likely(x)	__builtin_expect(((x)),1)
#endif

#ifndef unlikely
	#define unlikely(x)	__builtin_expect(((x)),0)
#endif

#endif /* LIKELY_H_ */
