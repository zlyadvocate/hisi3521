/*
 * avstream.h
 *
 *  Created on: May 4, 2014
 *      Author: root
 */

#ifndef AVSTREAM_H_
#define AVSTREAM_H_

#define EXAMPLE_PORT 1007
#define GROUPIP "239.0.0.168"
#define VideoBasePort 1000
#define VideoPackLength 512


#define AudioBasePort 2001
#define VideoBufferlength 32*1024   //64*1024 before
#define AudioFramePoint 160
#define AudioBufferlength AudioFramePoint*2
//AAC frame length
#define FAACFRAMELEN 960

#endif /* AVSTREAM_H_ */
