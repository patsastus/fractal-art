# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/26 09:12:21 by nraatika          #+#    #+#              #
#    Updated: 2025/06/17 10:43:22 by nraatika         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:= fractol

SRC_DIR		:= src/c
OBJ_DIR		:= objs
INC_DIR		:= include

SRCS		:= complex.c init.c main.c newton.c utils.c hooks.c julia.c mandelbrot.c newton_utils.c visuals.c

HEADER		:= fractol.h

OBJS		:= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))


CC			:= cc
CFLAGS		:= -Wall -Wextra -Werror -g
CFLAGS 		+= -O3 -ffast-math

RM			:= rm -f

MLX_PATH	:= ./MLX42/build/
MLX_NAME	:= libmlx42.a
MLX_BPATH	:= ./MLX42/
MLX			:= $(MLX_PATH)$(MLX_NAME)

INC			:= -I./MLX42/include/MLX42 -I./$(INC_DIR)
LIBS		:= -L$(MLX_PATH) -lmlx42 -lXext -lX11 -lm -lz -ldl -lglfw -pthread

$(NAME):			$(MLX) $(OBJ_DIR) $(OBJS) $(INC_DIR)/$(HEADER)
		$(CC) $(CFLAGS) $(INC) $(OBJS) $(LIBS) -o $(NAME)

$(OBJ_DIR)/%.o:		$(SRC_DIR)/%.c $(INC_DIR)/$(HEADER) $(MLX) 
		$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(MLX):		
		@if [ ! -d "$(MLX_BPATH)" ]; then \
			git clone -q --depth 1 --branch v2.4.1 --single-branch \
			https://github.com/codam-coding-college/MLX42.git; \
		fi
		cd $(MLX_BPATH)	&& cmake -B build
		$(MAKE) -C $(MLX_PATH)

$(OBJ_DIR):
		@mkdir -p $(OBJ_DIR)

all:				$(NAME)

clean:                                   
		$(RM) $(OBJS)
                                         
fclean:			clean 
		$(RM) $(NAME)

mlxclean:		
		$(RM) -r $(MLX_BPATH)

re:				fclean all

.PHONY:		all clean fclean re mlxclean
