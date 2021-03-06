/*++

          ##     ## ######## ##     ## ########  
          ##     ## ##       ##     ## ##     ## 
          ##     ## ##       ##     ## ##     ## 
          ######### ######   ##     ## ##     ## 
          ##     ## ##        ##   ##  ##     ## 
          ##     ## ##         ## ##   ##     ## 
          ##     ## ########    ###    ########  

            HackSys Extreme Vulnerable Driver

Author : Ashfaq Ansari
Contact: ashfaq[at]payatu[dot]com
Website: http://www.payatu.com/

Copyright (C) 2011-2016 Payatu Technologies Pvt. Ltd. All rights reserved.

This program is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either version
3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

See the file 'LICENSE' for complete copying permission.

Module Name:
    UninitializedVariable.c

Abstract:
    This module implements the functions to demonstrate
    use of Uninitialized Variable vulnerability.

--*/

#include "UninitializedVariable.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, TriggerUninitializedVariable)
    #pragma alloc_text(PAGE, UninitializedVariableIoctlHandler)
    #pragma alloc_text(PAGE, UninitializedVariableObjectCallback)
#endif // ALLOC_PRAGMA

#pragma auto_inline(off)

/// <summary>
/// Uninitialized Variable Object Callback
/// </summary>
VOID UninitializedVariableObjectCallback() {
    PAGED_CODE();

    DbgPrint("[+] Uninitialized Variable Object Callback called\n");
}

/// <summary>
/// Trigger the Uninitialized Variable Vulnerability
/// </summary>
/// <param name="UserBuffer">The pointer to user mode buffer</param>
/// <returns>NTSTATUS</returns>
NTSTATUS TriggerUninitializedVariable(IN PVOID UserBuffer) {
    ULONG UserValue = 0;
    ULONG MagicValue = 0xBAD0B0B0;
    NTSTATUS Status = STATUS_SUCCESS;

#ifdef SECURE
    // Secure Note: This is secure because the developer is properly initializing
    // UNINITIALIZED_VARIABLE to NULL and checks for NULL pointer before calling
    // the callback
    UNINITIALIZED_VARIABLE UninitializedVariable = {0};
#else
    // Vulnerability Note: This is a vanilla Uninitialized Variable vulnerability
    // because the developer is not initializing 'UNINITIALIZED_VARIABLE' structure
    // before calling the callback when 'MagicValue' does not match 'UserValue'
    UNINITIALIZED_VARIABLE UninitializedVariable;
#endif

    PAGED_CODE();

    __try {
        // Verify if the buffer resides in user mode
        ProbeForRead(UserBuffer,
                     sizeof(UNINITIALIZED_VARIABLE),
                     (ULONG)__alignof(UNINITIALIZED_VARIABLE));

        // Get the value from user mode
        UserValue = *(PULONG)UserBuffer;

        DbgPrint("[+] UserValue: 0x%p\n", UserValue);
        DbgPrint("[+] UninitializedVariable Address: 0x%p\n", &UninitializedVariable);

        // Validate the magic value
        if (UserValue == MagicValue) {
            UninitializedVariable.Value = UserValue;
            UninitializedVariable.Callback = &UninitializedVariableObjectCallback;
        }

        DbgPrint("[+] UninitializedVariable.Value: 0x%p\n", UninitializedVariable.Value);
        DbgPrint("[+] UninitializedVariable.Callback: 0x%p\n", UninitializedVariable.Callback);

#ifndef SECURE
        DbgPrint("[+] Triggering Uninitialized Variable Vulnerability\n");
#endif

        // Call the callback function
        if (UninitializedVariable.Callback) {
            UninitializedVariable.Callback();
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        DbgPrint("[-] Exception Code: 0x%X\n", Status);
    }

    return Status;
}

/// <summary>
/// Uninitialized Variable Ioctl Handler
/// </summary>
/// <param name="Irp">The pointer to IRP</param>
/// <param name="IrpSp">The pointer to IO_STACK_LOCATION structure</param>
/// <returns>NTSTATUS</returns>
NTSTATUS UninitializedVariableIoctlHandler(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp) {
    PVOID UserBuffer = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    UNREFERENCED_PARAMETER(Irp);
    PAGED_CODE();

    UserBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    if (UserBuffer) {
        Status = TriggerUninitializedVariable(UserBuffer);
    }

    return Status;
}

#pragma auto_inline()
