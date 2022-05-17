int exp_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == ZR && rhs == ZR) return TRUE;
	if(lhs == G1 && rhs == ZR) return TRUE;
	if(lhs == G2 && rhs == ZR) return TRUE;
	if(lhs == GT && rhs == ZR) return TRUE;
	return FALSE; /* Fail all other cases */
}

int mul_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs) return TRUE;
	if(lhs == ZR || rhs == ZR) return TRUE;
	return FALSE; /* Fail all other cases */
}

int add_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs && lhs != GT) return TRUE;
	return FALSE; /* Fail all other cases */
}

int sub_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs && lhs != GT) return TRUE;
	return FALSE; /* Fail all other cases */
}

int div_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs) return TRUE;
	return FALSE; /* Fail all other cases */
}

int pair_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == G1 && rhs == G2) return TRUE;
	else if(lhs == G2 && rhs == G1) return TRUE;
	return FALSE; /* Fall all other cases: only for MNT case */
}

int check_type(GroupType type) {
	if(type == ZR || type == G1 || type == G2 || type == GT) return TRUE;
	return FALSE;
}
