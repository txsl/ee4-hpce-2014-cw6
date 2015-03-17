
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

// ; __kernel void kernel_update(const float inner, const float outer, __global const float *world_state, __global  const uint *packed_properties, __global float *buffer)
// ; {
// ;     uint x=get_global_id(0);
// ;     uint y=get_global_id(1);
// ;     uint w=get_global_size(0);

// ;     unsigned index=y*w + x;

// ;     unsigned packed_value = packed_properties[index];
            
// ;     if((packed_value & Cell_Fixed) || (packed_value & Cell_Insulator)){
// ;         // Do nothing, this cell never changes (e.g. a boundary, or an interior fixed-value heat-source)
// ;         buffer[index]=world_state[index];
// ;     }else{
// ;         float contrib=inner;
// ;         float acc=inner*world_state[index];
        
// ;         // Cell above
// ;         if(! (packed_value & Cell_Above)) {
// ;             contrib += outer;
// ;             acc += outer * world_state[index-w];
// ;         }
        
// ;         // Cell below
// ;         if(! (packed_value & Cell_Below)) {
// ;             contrib += outer;
// ;             acc += outer * world_state[index+w];
// ;         }
        
// ;         // Cell left
// ;         if(! (packed_value & Cell_Left)) {
// ;             contrib += outer;
// ;             acc += outer * world_state[index-1];
// ;         }
        
// ;         // Cell right
// ;         if(! (packed_value & Cell_Right)) {
// ;             contrib += outer;
// ;             acc += outer * world_state[index+1];
// ;         }
        
// ;         // Scale the accumulate value by the number of places contributing to it
// ;         float res=acc/contrib;
// ;         // Then clamp to the range [0,1]
// ;         res = min(1.0f, max(0.0f, res));
// ;         buffer[index] = res;
// ;     }
// ; };