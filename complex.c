/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   complex.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 10:21:49 by nraatika          #+#    #+#             */
/*   Updated: 2025/05/26 11:07:53 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void 	ft_mult(t_comp *res, t_comp *one, t_comp *two)
{
	t_comp	temp;

	temp.re = (one->re) * (two->re) - (one->im) * (two->im);
	temp.im = (one->re) * (two->im) + (one->im) * (two->re);
	res->re = temp.re;
	res->im = temp.im;
}

void	ft_add(t_comp *res, t_comp *one, t_comp *two)
{
	t_comp	temp;	

	temp.re = (one->re) + (two->re);
	temp.im = (one->im) + (two->im);
	res->re = temp.re;
	res->im = temp.im;
}

double	ft_abs(t_comp *z)
{
	return (sqrt((z->re) * (z->re) + (z->im) * (z->im)));
}
