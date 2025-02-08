if ((i < nb_ref_pics) && (i<16))
					deltaPOC += ref_ps->delta_poc[i];

				if (k<16)
					rps->delta_poc[k] = deltaPOC;

				if (deltaPOC < 0)  k0++;
				else k1++;

				k++;