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
BONUS		:= .bonus

SRC_DIR		:= srcs
OBJ_DIR		:= objs
BONUS_DIR	:= bonus
B_OBJ_DIR	:= bonus_objs
INC_DIR		:= include

SRCS		:=	main.c	utils.c	hooks.c	init.c	complex.c	mandelbrot.c\
			visuals.c	julia.c

B_SRCS		:= complex_bonus.c	init_bonus.c	main_bonus.c	newton_bonus.c\
			  utils_bonus.c	hooks_bonus.c	 julia_bonus.c	mandelbrot_bonus.c\
			  newton_utils_bonus.c	visuals_bonus.c

HEADER		:=	fractol.h
B_HEADER	:=	fractol_bonus.h

OBJS		:= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))
B_OBJS		:= $(addprefix $(B_OBJ_DIR)/, $(B_SRCS:.c=.o))


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

$(B_OBJ_DIR)/%.o:	$(BONUS_DIR)/%.c $(INC_DIR)/$(B_HEADER) $(MLX) 
		$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(MLX):		
		git clone -q --depth 1 --branch v2.4.1 --single-branch\
			https://github.com/codam-coding-college/MLX42.git
		cd $(MLX_BPATH)	&& cmake -B build
		$(MAKE) -C $(MLX_PATH)

$(BONUS):			$(MLX) $(B_OBJ_DIR) $(B_OBJS) $(INC_DIR)/$(B_HEADER)
		$(CC) $(CFLAGS) $(INC) $(B_OBJS) $(LIBS) -o $(NAME)
		touch $(BONUS)

$(B_OBJ_DIR):
		@mkdir -p $(B_OBJ_DIR)

$(OBJ_DIR):
		@mkdir -p $(OBJ_DIR)

all:				$(NAME)

clean:                                   
		$(RM) $(OBJS) $(B_OBJS)
                                         
fclean:			clean 
		$(RM) $(NAME) $(BONUS)

mlxclean:		
		$(RM) -r $(MLX42)

re:				fclean all

bonus:				$(BONUS)

.PHONY:		all clean fclean re mlxclean bonus
