/*****************************************************************************
MotionLink::AddForce
*****************************************************************************/

		if (m_rErrorI > 90)
			m_rErrorI = 90;
		if (m_rErrorI < -90)
			m_rErrorI = -90;

		// P, I, and D no longer apply
		// P is still sorta proportional
		// I determines fMax
		// D sets a minimum fMax
		real rVel = rErrorP * (m_rGainP / 100);
		real rForce = (m_rGainI) + m_rGainD;
		
		// Pseudo-PID method
		real rForce = (m_rGainP * (rErrorP / 100)) + (m_rGainI * (m_rErrorI / 1000));// + (m_rGainD * rErrorD);
		
		/// The "true PID" method... or something that looks close but doesn't work.
		real rErrorP = rDesiredAngle - pRevoluteJoint->rGetAngle ();
		m_rErrorI = m_rErrorI + rErrorP;

		real rForce = m_rLastForce + (m_rGainP * rErrorP); // + (m_rGainI * m_rErrorI) + (m_rGainD * rErrorD);
		m_rLastForce = rForce;

/****************************************************************************/
/** RevoluteJoint::GetAngle
*****************************************************************************/
		/*
		if (m_pBody1 && m_pBody2)
		{
			VectorXYZ vecBody1, vecBody2;
			VectorYPR vecBody1Rotation, vecBody2Rotation;
			m_pBody1->vGetRotation (vecBody1Rotation);
			m_pBody2->vGetRotation (vecBody2Rotation);

			if (m_eBody1Axis = aZ)
			{
				vecBody1 = VectorXYZ(0,0,1);
			}
			else if (m_eBody1Axis = aY)
			{
				vecBody1 = VectorXYZ(0,1,0);
			}

			if (m_eBody2Axis = aZ)
			{
				vecBody2 = VectorXYZ(0,0,1);
			}
			else if (m_eBody2Axis = aY)
			{
				vecBody2 = VectorXYZ(0,1,0);
			}

			vecBody1.vRotateByDegrees (vecBody1Rotation);
			vecBody2.vRotateByDegrees (vecBody2Rotation);

			rResult = rRadiansBetween (vecBody1, vecBody2);
			rResult *= Generic::rRadiansToDegrees;
		}
		else
			rResult = 0;
		*/

/*****************************************************************************
<FOOTER>
*****************************************************************************/
