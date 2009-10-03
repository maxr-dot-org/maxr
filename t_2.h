/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __T2_H
#define __T2_H

#include <math.h>

template <class Type> class T_2{
	public:
	static T_2<Type> Zero;
	Type
		x,y;
	T_2():x(0),y(0){}
	T_2(Type px, Type py):x(px),y(py){}
	void operator ()(const Type &PX,const Type &PY){
		x = PX;
		y = PY;
	}
	bool operator == (const T_2<Type> &P)const{
		return x == P.x && y == P.y;
	}
	bool operator != (const T_2<Type> &P)const{
		return x != P.x || y != P.y;
	}
	void operator *= (Type f){
		x *= f;
		y *= f;
	}
	void operator /= (Type f){
		Type
			f_ = 1.0f / f;
		x *= f_;
		y *= f_;
	}
/*	void operator *= (const T_2 &P){
		x *= P.x;
		y *= P.y;
	}
	void operator /= (const T_2 &P){
		x /= P.x;
		y /= P.y;
	}*/
	void operator += (const Type f){
		x += f;
		y += f;
	}
	void operator -= (const Type f){
		x -= f;
		y -= f;
	}
	void operator += (const T_2<Type> &P){
		x += P.x;
		y += P.y;
	}
	void operator -= (const T_2<Type> &P){
		x -= P.x;
		y -= P.y;
	}
	T_2<Type> operator + (const T_2<Type> &P)const{
		T_2<Type>
			R = *this;
		R.x += P.x;
		R.y += P.y;
		return R;
	}
	T_2<Type> operator - (const T_2<Type> &P)const{
		T_2<Type>
			R = *this;
		R.x -= P.x;
		R.y -= P.y;
		return R;
	}
	T_2<Type> operator - ()const{
		T_2<Type>
			R;
		R.x = Type(-x);
		R.y = Type(-y);
		return R;
	}
	void Int(){
		x = Type(int(x));
		y = Type(int(y));
	}
	void SetZero(){
		x = y = 0;
	}
	T_2<Type> operator * (Type f)const{
		T_2<Type>
			R;
		R(Type(x*f),Type(y*f));
		return R;
	}
/*
	T_2 operator * (const T_2<Type> &P2)const{
		T_2
			R;
		R(Type(x * P2.x),Type(y * P2.y));
		return R;
		}
*/
	T_2<Type> operator / (Type f){
		Type f_ = Type(1 / f);
		T_2<Type>
			R;
		R(Type(x*f_),Type(y*f_));
		return R;
	}
/*
	T_2 operator / (const T_2<Type> &P2){
		T_2
			R;
		R(Type(x / P2.x),Type(y / P2.y));
		return R;
	}*/
	Type operator * (const T_2<Type> &P)const{
		return Type(x*P.x + y*P.y);
	}
	Type scalarProduct (const T_2<Type> &A,const T_2<Type> &B)const{
		Type
			x = B.x - A.x,
			y = B.y - A.y;
		return Type(x*x + y*y);
	}
	void interpolate(const T_2<Type> &P1,const T_2<Type> &P2,Type f){
		x = Type(P1.x + (P2.x - P1.x)*f);
		y = Type(P1.y + (P2.y - P1.y)*f);
	}
	T_2<Type> interpolate(const T_2<Type> &P1,Type f){
		T_2<Type>
			R;
		R.x = Type(x + (P1.x - x)*f);
		R.y = Type(y + (P1.y - y)*f);
		return R;
	}
	T_2<Type> interpolate(const T_2<Type> &P1){
		T_2<Type>
			R;
		R.x = Type(x + (P1.x - x)*0.5);
		R.y = Type(y + (P1.y - y)*0.5);
		return R;
	}
	void Interpolate(const T_2<Type> &P1,const T_2<Type> &P2){
		x = (P1.x + P2.x)*0.5;
		y = (P1.y + P2.y)*0.5;
	}
	Type dist()const{
		return sqrt(x*x + y*y);
	}
	Type dist(const T_2<Type> &P)const{
		float
			DX = P.x-x,
			DY = P.y-y;
		return sqrt(DX*DX + DY*DY);
	}
	Type distSqr()const{
		return Type(x*x + y*y);
	}
	Type distSqr(const T_2<Type> &P)const{
		float
			DX = P.x-x,
			DY = P.y-y;
		return DX*DX + DY*DY;
	}
	void normalize(){
		float
			f = 1 / dist();
		x *= f;
		y *= f;
	}
	T_2<Type> normalized()const{
		T_2<Type>
			R = *this;
		R.normalize();
		return R;
	}
	T_2<Type> operator !()const{
		T_2<Type>
			R;
		R(Type(-y),Type(x));
		return R;
	}
/*	T_2<Type> GetOrthogonal()const{
		T_2<Type>
			R;
		R(Type(-y),Type(x));
		return R;
	}*/
	void setOrthogonal(const T_2<Type> &A,const T_2<Type> &B){
		x = A.y - B.y;
		y = B.x - A.x;
	}
	void setOrthonormal(const T_2<Type> &A,const T_2<Type> &B){
		x = A.y - B.y;
		y = B.x - A.x;
		Type
		   _f = 1./sqrt(x*x + y*y);
		x *= _f;
		y *= _f;
	}
};


template <class Type> inline T_2<Type> I_2(const Type PX,const Type PY){
	T_2<Type>
		P;
	P(PX,PY);
	return(P);
}

template <class Type> inline T_2<Type> operator * (Type f,const T_2<Type> &P){
	return(I_2(P.x*f,P.y*f));
}

template <class Type> T_2<Type> T_2<Type>::Zero(0,0);


#endif
