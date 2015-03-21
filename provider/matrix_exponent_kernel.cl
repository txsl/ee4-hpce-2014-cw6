__kernel void matrix_multiply(
  __global const int *vectorIn, 
  __global const int *matrixIn, 
  __global int *vectorOut)
{
    int r=get_global_id(0);
    int n=get_global_size(0);

    vectorOut[r] = 0;
    for (int i = 0; i < n; i++) {
      vectorOut[r] = (vectorOut[r] + ( ((uint)vectorIn[r] * (uint)matrixIn[i * n + r]) % 2147483647ULL) ) % 2147483647ULL;
    }

}