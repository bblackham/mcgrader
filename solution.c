static int solve_iter(int x, int count)
{
    if (x == 1)
        return count;
    if (x & 1)
        return solve_iter(3*x+1, count+1);
    else
        return solve_iter(x>>1, count+1);
}

int solve(int x)
{
    return solve_iter(x, 0);
}
