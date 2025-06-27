/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 11:02:55 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 17:01:48 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

/*
	Parses the given argument into a double. Needs checking for format first
*/
double	string_to_double(char *s)
{
	double	num;
	double	ret;
	int		sign;

	sign = 1;
	if (*s == '-')
	{
		sign = -1;
		++s;
	}
	ret = (*s - '0') * 1.0;
	s = s + 2;
	num = 10.0;
	while (*s)
	{
		ret += (*s - '0') / num;
		num *= 10;
		++s;
	}
	return (ret * sign);
}

/*
	Check that input argument is of required format: optional leading minus,
	followed by a digit between 0 and 2, followed by a dot, followed by 1+ 
	digits until end of string.
*/
int	check_input_arg(char *s)
{
	if (s && *s == '-')
		++s;
	if (!(*s) || !(*s >= '0' && *s <= '2'))
		return (0);
	++s;
	if (!(*s) || *s != '.')
		return (0);
	++s;
	if (!(*s) || !(*s >= '0' && *s <= '9'))
		return (0);
	++s;
	while (*s)
	{
		if (!(*s >= '0' && *s <= '9'))
			return (0);
		++s;
	}
	return (1);
}

//Writes usage instructions to stdout
void	write_instructions(void)
{
	const char	*message = "Please provide valid input:\n";
	const char	*format = "./fractol [jmn] -*[0-2].[0-9]* -*[0-2].[0-9]*\n";
	const char	*ex1 = "Example:\t./fractol m\n";
	const char	*ex2 = "\t\t./fractol j -0.4 0.6\n";
	const char	*ex3 = "Example:\t./fractol n\n";

	write(1, message, ft_strlen(message));
	write(1, format, ft_strlen(format));
	write(1, ex1, ft_strlen(ex1));
	write(1, ex2, ft_strlen(ex2));
	write(1, ex3, ft_strlen(ex3));
}

//make a sawtooth pattern: 2 peaks 2 valleys.
uint32_t	sawtooth(uint32_t i, uint32_t max)
{
	if (i % (max / 2) < max / 4)
		return (i % (max / 4));
	else
		return (max / 4 - (i % (max / 4)));
}

size_t	ft_strlen(const char *s)
{
	size_t	size;

	size = 0;
	while (*s)
	{
		++size;
		++s;
	}
	return (size);
}
