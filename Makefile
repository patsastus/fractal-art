# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/26 09:12:21 by nraatika          #+#    #+#              #
#    Updated: 2025/05/27 12:22:07 by nraatika         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS	=fract-ol.c complex.c julia.c

HEADER	=fractol.h

OBJS	=$(SRCS:%.c=%.o)

NAME	=fractol

CC		=cc
CFLAGS	=-Wall -Wextra -Werror -Imlx_linux -O3
RM		=rm -f
MLX_DIR	=minilibx-linux
MLX		=$(MLX_DIR)/libmlx_Linux.a

$(NAME):		$(OBJS) $(HEADER) $(MLX)
	$(CC) $(OBJS) -L$(MLX_DIR) -lmlx_Linux -lXext -lX11 -lm -lz -o $(NAME)

$(OBJS:%.o):	$(SRCS:%.c) $(HEADER)
	$(CC) $(CFLAGS) $(HEADER) -c $< -o $@

$(MLX):			
	wget https://cdn.intra.42.fr/document/document/32351/minilibx-linux.tgz
	tar -xvf minilibx-linux.tgz
	$(MAKE) -C $(MLX_DIR)
