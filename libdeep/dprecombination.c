/***************************************************************************
 *            dprecombination.c
 *
 *  Fri Mar 23 15:39:19 2012
 *  Copyright  2012  Konstantin Kozlov
 *  <kozlov@spbcas.ru>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glib.h>
#include "dpindivid.h"
#include "dppopulation.h"
#include "dprecombination.h"

void dp_individ_recombination(DpRecombinationControl *control, GRand*hrand, DpIndivid*individ,  DpIndivid*input_1,  DpIndivid*input_2,  DpIndivid*input_3,  DpIndivid*input_4, int start_index, int end_index)
{
	int i;
	int L;
	int flag;
	double u, phi;
	switch (control->strategy) {
		case Simple:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				if ( g_rand_double(hrand) < control->p[i] || L == 0 ) {
					individ->x[i] = input_1->x[i] + control->f[i] * input_2->x[i];
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_bin:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				if ( g_rand_double(hrand) < control->p[i] || L == 0 ) {
					individ->x[i] = input_1->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_exp:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				individ->x[i] = input_1->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				if ( g_rand_double(hrand) > control->p[i] )
					break;
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_bin_rand:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				if ( g_rand_double(hrand) < control->p[i] || L == 0 ) {
					individ->x[i] = input_4->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_exp_rand:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				individ->x[i] = input_4->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				if ( g_rand_double(hrand) > control->p[i] )
					break;
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_bin_self:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				if ( g_rand_double(hrand) < control->p[i] || L == 0 ) {
					individ->x[i] = individ->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_exp_self:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				individ->x[i] = individ->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				if ( g_rand_double(hrand) > control->p[i] )
					break;
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_bin_T:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				u = g_rand_double(hrand);
				if ( u < control->p[i] ) {
					individ->x[i] = input_1->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				} else if ( u < 1 - control->p[i] ) {
					phi = input_1->cost + input_2->cost + input_3->cost;
					individ->x[i] = ( input_2->cost - input_1->cost ) * ( input_1->x[i] - input_2->x[i] );
					individ->x[i] += ( input_3->cost - input_2->cost ) * ( input_2->x[i] - input_3->x[i] );
					individ->x[i] += ( input_1->cost - input_3->cost ) * ( input_3->x[i] - input_1->x[i] );
					phi = ( phi <  0 ) ? -phi : phi;
                    individ->x[i] = ( phi ==  0 ) ? individ->x[i] : individ->x[i]/phi;
					individ->x[i] += ( input_1->x[i] + input_2->x[i] + input_3->x[i] ) / 3.0;
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_bin_rand_T:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				u = g_rand_double(hrand);
				if ( u < control->p[i] ) {
					individ->x[i] = input_4->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				} else if ( u < 1 - control->p[i] ) {
					phi = input_4->cost + input_2->cost + input_3->cost;
					individ->x[i] = ( input_2->cost - input_4->cost ) * ( input_4->x[i] - input_2->x[i] );
					individ->x[i] += ( input_3->cost - input_2->cost ) * ( input_2->x[i] - input_3->x[i] );
					individ->x[i] += ( input_4->cost - input_3->cost ) * ( input_3->x[i] - input_4->x[i] );
					phi = ( phi <  0 ) ? -phi : phi;
                    individ->x[i] = ( phi ==  0 ) ? individ->x[i] : individ->x[i]/phi;
					individ->x[i] += ( input_4->x[i] + input_2->x[i] + input_3->x[i] ) / 3.0;
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_bin_self_T:
			i = start_index;
			for ( L = 0; L < end_index; L++ ) {
				u = g_rand_double(hrand);
				if ( u < control->p[i] ) {
					individ->x[i] = individ->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
				} else if ( u < 1 - control->p[i] ) {
					double indx = individ->x[i];
					phi = individ->cost + input_2->cost + input_3->cost;
					individ->x[i] = ( input_2->cost - individ->cost ) * ( indx - input_2->x[i] );
					individ->x[i] += ( input_3->cost - input_2->cost ) * ( input_2->x[i] - input_3->x[i] );
					individ->x[i] += ( individ->cost - input_3->cost ) * ( input_3->x[i] - indx );
					phi = ( phi <  0 ) ? -phi : phi;
                    individ->x[i] = ( phi ==  0 ) ? individ->x[i] : individ->x[i]/phi;
					individ->x[i] += ( indx + input_2->x[i] + input_3->x[i] ) / 3.0;
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;					
		case DE_3_exp_T:
			i = start_index;
			flag = 1;
			for ( L = 0; L < end_index; L++ ) {
				if ( flag == 1 ) {
					individ->x[i] = input_1->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
					if ( g_rand_double(hrand) > control->p[i] )
						flag = 0;
				} else {
					phi = input_1->cost + input_2->cost + input_3->cost;
					individ->x[i] = ( input_2->cost - input_1->cost ) * ( input_1->x[i] - input_2->x[i] );
					individ->x[i] += ( input_3->cost - input_2->cost ) * ( input_2->x[i] - input_3->x[i] );
					individ->x[i] += ( input_1->cost - input_3->cost ) * ( input_3->x[i] - input_1->x[i] );
					phi = ( phi <  0 ) ? -phi : phi;
                    individ->x[i] = ( phi ==  0 ) ? individ->x[i] : individ->x[i]/phi;
					individ->x[i] += ( input_1->x[i] + input_2->x[i] + input_3->x[i] ) / 3.0;
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_exp_rand_T:
			i = start_index;
			flag = 1;
			for ( L = 0; L < end_index; L++ ) {
				if ( flag == 1 ) {
					individ->x[i] = input_4->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
					if ( g_rand_double(hrand) > control->p[i] )
						flag = 0;
				} else {
					phi = input_4->cost + input_2->cost + input_3->cost;
					individ->x[i] = ( input_2->cost - input_4->cost ) * ( input_4->x[i] - input_2->x[i] );
					individ->x[i] += ( input_3->cost - input_2->cost ) * ( input_2->x[i] - input_3->x[i] );
					individ->x[i] += ( input_4->cost - input_3->cost ) * ( input_3->x[i] - input_4->x[i] );
					phi = ( phi <  0 ) ? -phi : phi;
                    individ->x[i] = ( phi ==  0 ) ? individ->x[i] : individ->x[i]/phi;
					individ->x[i] += ( input_4->x[i] + input_2->x[i] + input_3->x[i] ) / 3.0;
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		case DE_3_exp_self_T:
			i = start_index;
			flag = 1;
			for ( L = 0; L < end_index; L++ ) {
				if ( flag == 1 ) {
					individ->x[i] = individ->x[i] + control->f[i] * ( input_2->x[i] - input_3->x[i] );
					if ( g_rand_double(hrand) > control->p[i] )
						flag = 0;
				} else {
					double indx = individ->x[i];
					phi = individ->cost + input_2->cost + input_3->cost;
					individ->x[i] = ( input_2->cost - individ->cost ) * ( indx - input_2->x[i] );
					individ->x[i] += ( input_3->cost - input_2->cost ) * ( input_2->x[i] - input_3->x[i] );
					individ->x[i] += ( individ->cost - input_3->cost ) * ( input_3->x[i] - indx );
					phi = ( phi <  0 ) ? -phi : phi;
                    individ->x[i] = ( phi ==  0 ) ? individ->x[i] : individ->x[i]/phi;
					individ->x[i] += ( indx + input_2->x[i] + input_3->x[i] ) / 3.0;
				}
				i++;
				if ( i > end_index - 1 ) {
					i = 0;
				}
			}
		break;
		default:
			fprintf(stderr, "\ndp_individ_recombination: unknown type %d\n", control->strategy);
			exit(-1);
	}
}

DpRecombinationControl*dp_recombination_control_init(DpRecombinationStrategy strategy, DpPopulation*pop, GRand*hrand, double weight, double prob, double gamma)
{
	DpRecombinationControl*rc;
	int i;
	rc = (DpRecombinationControl*)malloc(sizeof(DpRecombinationControl));
	rc->strategy = strategy;
	rc->size = pop->ind_size;
	rc->pop_size = pop->size;
	rc->toggle = 0;
	rc->gamma = gamma;
	rc->p_inf = G_MINDOUBLE;
	rc->p_supp = 1.0;
	rc->f_inf = 1.0 / sqrt((double)pop->ind_size);
	rc->f_supp = 2.0;
	rc->f = (double*)calloc(rc->size, sizeof(double));
	rc->p = (double*)calloc(rc->size, sizeof(double));
	rc->c = (double*)calloc(rc->size, sizeof(double));
	rc->v = (double*)calloc(rc->size, sizeof(double));
	if ( weight == 0 && prob == 0 && gamma > 0 ) {
		rc->adjust = 1;
		for ( i = 0; i < rc->size; i++ ) {
			rc->f[i] = g_rand_double_range(hrand, rc->f_inf, rc->f_supp);
			rc->p[i] = g_rand_double_range(hrand, rc->p_inf, rc->p_supp);
			rc->v[i] = pop->variance[i];
		}
	} else {
		rc->adjust = 0;
		for ( i = 0; i < rc->size; i++ ) {
			rc->f[i] = weight;
			rc->p[i] = prob;
			rc->v[i] = pop->variance[i];
		}
	}
	return rc;
}

void dp_recombination_control_save(FILE*fp, DpRecombinationControl*rc)
{
	int i;
	for ( i = 0; i < rc->size; i++ ) {
	  fprintf(fp, "%13.9f ", rc->f[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < rc->size; i++ ) {
	  fprintf(fp, "%13.9f ", rc->p[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < rc->size; i++ ) {
	  fprintf(fp, "%13.9f ", rc->v[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < rc->size; i++ ) {
	  fprintf(fp, "%13.9f ", rc->c[i]);
	}
}

void dp_recombination_control_load(FILE*fp, DpRecombinationControl*rc)
{
	int i;
	for ( i = 0; i < rc->size; i++ ) {
	  fscanf(fp, "%lf", &(rc->f[i]));
	}
	for ( i = 0; i < rc->size; i++ ) {
	  fscanf(fp, "%lf", &(rc->p[i]));
	}
	for ( i = 0; i < rc->size; i++ ) {
	  fscanf(fp, "%lf", &(rc->v[i]));
	}
	for ( i = 0; i < rc->size; i++ ) {
	  fscanf(fp, "%lf", &(rc->c[i]));
	}
}

void dp_recombination_control_renew(DpRecombinationControl*rc, GRand*hrand, DpPopulation*pop)
{
	int i;
	if ( rc->adjust == 1 ) {
		for ( i = 0; i < rc->size; i++ ) {
			rc->f[i] = g_rand_double_range(hrand, rc->f_inf, rc->f_supp);
			rc->p[i] = g_rand_double_range(hrand, rc->p_inf, rc->p_supp);
			rc->v[i] = pop->variance[i];
		}
	}
}

void dp_recombination_control_update(DpRecombinationControl*rc, GRand*hrand, DpPopulation*pop, int start_index, int end_index)
{
	int i, j;
	double flag;
	if ( rc->adjust == 0 )
		return;
	for ( i = 0; i < rc->size; i++ ) {
		if ( pop->variance[i] > 0 )
			rc->c[i] = rc->gamma * rc->v[i] / pop->variance[i];
		else
			rc->c[i] = 0.0;
		rc->v[i] = pop->variance[i];
	}
	if ( rc->toggle == 1 ) {
		rc->toggle = 0;
		for ( j = 0; j < rc->size; j++ ) {
			flag = rc->pop_size * rc->f[j] * rc->f[j] - 1.0;
			rc->p[j] = ( rc->c[j] < 1 ) ? rc->p_inf : -flag + sqrt ( flag * flag - rc->pop_size * ( 1.0 - rc->c[j] ) );
			if ( ( rc->p[j] >= rc->p_supp ) || ( rc->p[j] <= rc->p_inf ) )
				rc->p[j] = g_rand_double_range(hrand, rc->p_inf, rc->p_supp);
		}
	} else {
		rc->toggle = 1;
		for ( j = 0; j < rc->size; j++ ) {
			flag = rc->pop_size * ( rc->c[j] - 1.0 ) + rc->p[j] * ( rc->p_supp + 1 - rc->p[j] );
			rc->f[j] = ( flag < 0 ) ? rc->f_inf : sqrt ( flag / ( 2.0 * rc->pop_size * rc->p[j] ) );
			if ( ( rc->f[j] >= rc->f_supp ) || ( rc->f[j] <= rc->f_inf ) )
				rc->f[j] = g_rand_double_range(hrand, rc->f_inf, rc->f_supp);
		}
	}
}
