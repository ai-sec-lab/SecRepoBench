if ((double) x > p->bounds.x2)
      {
        winding_number+=p->direction ? 1 : -1;
        continue;
      }
    i=(ssize_t) MagickMax((double) p->highwater,1.0);
    for ( ; i < (ssize_t) (p->number_points-1); i++)
      if ((double) y <= p->points[i].y)
        break;
    q=p->points+i-1;