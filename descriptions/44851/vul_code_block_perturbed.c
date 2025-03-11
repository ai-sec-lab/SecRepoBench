if ((double) ycoordinate <= p->bounds.y1)
      break;
    if (((double) ycoordinate > p->bounds.y2) || ((double) x <= p->bounds.x1))
      continue;
    if ((double) x > p->bounds.x2)
      {
        winding_number+=p->direction != 0 ? 1 : -1;
        continue;
      }
    i=(ssize_t) MagickMax((double) p->highwater,1.0);
    for ( ; i < (ssize_t) p->number_points; i++)
      if ((double) ycoordinate <= p->points[i].y)
        break;
    q=p->points+i-1;
    if ((((q+1)->x-q->x)*(ycoordinate-q->y)) <= (((q+1)->y-q->y)*(x-q->x)))
      winding_number+=p->direction != 0 ? 1 : -1;