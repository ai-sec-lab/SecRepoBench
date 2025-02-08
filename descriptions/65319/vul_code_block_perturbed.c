if (i < nb_ref_pics)
					deltaPOC += ref_ps->delta_poc[i];

				rps->delta_poc[piccount] = deltaPOC;

				if (deltaPOC < 0)  k0++;
				else k1++;

				piccount++;