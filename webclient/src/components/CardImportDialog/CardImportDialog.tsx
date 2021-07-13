import React, { useState } from "react";
import Dialog from "@material-ui/core/Dialog";

import { CardImportForm } from 'forms';

interface CardImportDialogProps {
  isOpen: boolean;
  handleClose: Function;
}

const CardImportDialog = ({ handleClose, isOpen }: CardImportDialogProps) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <CardImportForm></CardImportForm>
    </Dialog>
  );
};

export default CardImportDialog;