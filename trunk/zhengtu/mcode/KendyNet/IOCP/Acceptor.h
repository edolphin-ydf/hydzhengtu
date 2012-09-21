/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/	
#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H
#include <stdint.h>
typedef struct acceptor* acceptor_t;
typedef void (*on_accept)(SOCKET,void*ud);
acceptor_t create_acceptor(const char *ip,uint32_t port,on_accept accept_callback,void*ud);
void       destroy_acceptor(acceptor_t*);
void       acceptor_run(acceptor_t,int32_t ms);

#endif