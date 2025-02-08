if ((i < nb_ref_pics) && (i<16))
					deltaPOC += ref_ps->delta_poc[i];

				if (piccount<16)
					rps->delta_poc[piccount] = deltaPOC;

				if (deltaPOC < 0)  k0++;
				else k1++;

				piccount++;