
__kernel void life_update(const int n, __global const int *curr, __global int *next)
{
    int x=get_global_id(0);
    int y=get_global_id(1);

    int result;

    int neighbours=0;
    for(int dx=-1; dx<=+1; dx++){
      for(int dy=-1; dy<=+1; dy++){
        int ox=(n+x+dx)%n; // handle wrap-around
        int oy=(n+y+dy)%n;

        if(curr[oy*n+ox] && !(dx==0 && dy==0))
          neighbours++;
      }
    }

    if(curr[n*y+x]){
      // alive
      if(neighbours<2){
        result = 0;
      }else if(neighbours>3){
        result = 0;
      }else{
        result = 1;
      }
    }else{
      // dead
      if(neighbours==3){
        result = 1;
      }else{
        result = 0;
      }
    }
    next[y*n+x] = result;
}

