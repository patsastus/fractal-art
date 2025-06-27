/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 14:20:43 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 15:49:56 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol.h>

/*
	Parse input, initialize data structures, set hooks, and then draw the first 
	image before starting the MLX-loop
*/
int	main(int argc, char **argv)
{
	t_data	data;

	data.error = 0;
	parse_params(&data, argc, argv);
	if (data.error)
	{
		write_instructions();
		return (1);
	}
	init_all(&data);
	if (mlx_image_to_window(data.mlx, data.img, OFFS, OFFS) == -1)
		ft_exit((void *)&data);
	mlx_scroll_hook(data.mlx, handle_mouse, &data);
	mlx_key_hook(data.mlx, handle_keys, &data);
	mlx_close_hook(data.mlx, ft_exit, &data);
	draw_fractal(&data);
	mlx_loop(data.mlx);
	ft_exit((void *)&data);
	return (0);
}

//something went wrong, exit after freeing 
void	ft_exit(void *param)
{
	t_data	*data;

	data = param;
	free(data->f.colors);
	mlx_close_window(data->mlx);
	mlx_terminate(data->mlx);
	exit((int)(data->error));
}
