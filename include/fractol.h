/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractol.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 14:22:20 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 15:49:29 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef FRACTOL_H
# define FRACTOL_H

# include <MLX42.h>
# include <math.h>
# include <stdlib.h>
# include <unistd.h>

# define MAX_ITERS 100
# define RESOLUTION 700
# define OFFS 100
# define COL_DEPTH 255

typedef struct s_complex
{
	double	re;
	double	im;
}	t_complex;

typedef struct s_anchor
{
	t_complex	value;
	int32_t		x;
	int32_t		y;
	double		px_step;
}	t_anchor;

typedef struct s_fractal
{
	t_complex	c;
	t_complex	z;
	double		r;
	uint32_t	max_iters;
	uint32_t	*colors;
	size_t		size;
}	t_fractal;

typedef struct s_data
{
	mlx_t			*mlx;
	mlx_image_t		*img;
	mlx_image_t		*text;
	t_fractal		f;
	t_anchor		a;
	double			zoom;
	uint32_t		height;
	uint32_t		width;
	uint32_t		img_h;
	uint32_t		img_w;
	char			error;
	uint32_t		(*iterator)(t_complex *, t_fractal *);
}	t_data;

//main.c
void		ft_exit(void *param);

//init.c
void		init_anchor(t_anchor *a, uint32_t w, uint32_t h, double scale);
void		init_all(t_data *data);
void		parse_params(t_data *data, int argc, char **argv);
void		set_sizes(t_data *data);
void		make_colors(t_fractal *f);

//complex.c
void		complex_mult(t_complex *res, t_complex *one, t_complex *two);
void		complex_div(t_complex *res, t_complex *num, t_complex *den);
double		complex_abs(t_complex *z);
void		set_complex(t_complex *z, double re, double im);
void		pixel_to_complex(t_complex *z, int32_t x, int32_t y, t_anchor *a);

//julia.c
uint32_t	iter_julia(t_complex *z, t_fractal *f);
double		calc_julia_radius(t_complex *c);
void		init_julia(t_data *data, int argc, char **argv);

//utils.c
double		string_to_double(char *s);
int			check_input_arg(char *s);
void		write_instructions(void);
uint32_t	sawtooth(uint32_t i, uint32_t max);
size_t		ft_strlen(const char *s);

//hooks.c
void		handle_keys(mlx_key_data_t keydata, void *param);
void		handle_mouse(double xdelta, double ydelta, void *param);

//mandelbrot.c
uint32_t	iter_mandelbrot(t_complex *z, t_fractal *f);
void		init_mandelbrot(t_data *data);

//visuals.c
void		reset_view(void *p);
void		make_text(t_data *data, int32_t x, int32_t y);
void		draw_fractal(void *p);

#endif
