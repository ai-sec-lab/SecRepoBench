CODEVIEW_INFO *
_bfd_XXi_slurp_codeview_record (bfd * abfd, file_ptr where, unsigned long length, CODEVIEW_INFO *cvinfo)
{
  char buffer[256+1];
  // <MASK>

  cvinfo->CVSignature = H_GET_32 (abfd, buffer);
  cvinfo->Age = 0;

  if ((cvinfo->CVSignature == CVINFO_PDB70_CVSIGNATURE)
      && (length > sizeof (CV_INFO_PDB70)))
    {
      CV_INFO_PDB70 *cvinfo70 = (CV_INFO_PDB70 *)(buffer);

      cvinfo->Age = H_GET_32(abfd, cvinfo70->Age);

      /* A GUID consists of 4,2,2 byte values in little-endian order, followed
	 by 8 single bytes.  Byte swap them so we can conveniently treat the GUID
	 as 16 bytes in big-endian order.  */
      bfd_putb32 (bfd_getl32 (cvinfo70->Signature), cvinfo->Signature);
      bfd_putb16 (bfd_getl16 (&(cvinfo70->Signature[4])), &(cvinfo->Signature[4]));
      bfd_putb16 (bfd_getl16 (&(cvinfo70->Signature[6])), &(cvinfo->Signature[6]));
      memcpy (&(cvinfo->Signature[8]), &(cvinfo70->Signature[8]), 8);

      cvinfo->SignatureLength = CV_INFO_SIGNATURE_LENGTH;
      /* cvinfo->PdbFileName = cvinfo70->PdbFileName;  */

      return cvinfo;
    }
  else if ((cvinfo->CVSignature == CVINFO_PDB20_CVSIGNATURE)
	   && (length > sizeof (CV_INFO_PDB20)))
    {
      CV_INFO_PDB20 *cvinfo20 = (CV_INFO_PDB20 *)(buffer);
      cvinfo->Age = H_GET_32(abfd, cvinfo20->Age);
      memcpy (cvinfo->Signature, cvinfo20->Signature, 4);
      cvinfo->SignatureLength = 4;
      /* cvinfo->PdbFileName = cvinfo20->PdbFileName;  */

      return cvinfo;
    }

  return NULL;
}