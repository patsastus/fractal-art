/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractol.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 14:22:20 by nraatika          #+#    #+#             */
/*   Updated: 2025/05/27 12:35:17 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef FRACTOL_H
# define FRACTOL_H

# include <mlx.h>
# include <math.h>
# include <stdlib.h>

# define MAX_ITER 100

typedef struct	s_image {
	void	*img;
	char	*addr;
	int		bpp;
	int		linelen;
	int		endian;
}	t_image;

typedef struct	s_comp {
	double	re;
	double	im;
}	t_comp;

typedef struct	s_anchor {
	t_comp	value;
	int		x;
	int		y;
	double	step;
	int		height;
	int		width;
}	t_anchor;

typedef struct	s_param {
	t_comp	c;
	double	r;
}	t_param;

void			make_image(t_image *im, int h, int w, void *mlx);
void			set_pixel(t_image *im, int x, int y, unsigned int color);
void			pixel_to_z(t_comp *z, int x, int y, t_anchor *a);
void			run_fractal(void *mlx, void *mlx_win);
void			init_anchor(t_anchor *a, int w, int h, double scale);

void		 	ft_mult(t_comp *res, t_comp *one, t_comp *two);
void			ft_add(t_comp *res, t_comp *one, t_comp *two);
double			ft_abs(t_comp *z);

int 			evaluate(t_comp *z, t_comp *c, double r);
t_param			*get_params(void);
void			new_image(t_image *im, t_anchor *a);
double			calc_r(t_comp *c);
unsigned int	get_color(int i);

#endif

